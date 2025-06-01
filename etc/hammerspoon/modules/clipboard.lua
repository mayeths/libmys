-- Clipboard History

local MAX_HISTORY_SIZE = 1000
local CHOOSER_WIDTH = 35
-- local LARGE_TEXT_THRESHOLD = 1 * 1024 * 1024
local LARGE_TEXT_THRESHOLD = 1 * 1024

local DATA_DIR = os.getenv("HOME") .. "/.cache/clipboard"
local STORAGE_DIR = DATA_DIR .. "/storage"
local G_PATH = DATA_DIR .. "/data.json"

-- https://developer.apple.com/library/archive/documentation/Miscellaneous/Reference/UTIRef/Articles/System-DeclaredUniformTypeIdentifiers.html
local UTI_TYPE = {
    IMAGE_TIFF = "public.tiff",
    IMAGE_PNG = "public.png",
    IMAGE_JPEG = "public.jpeg",
    PLAIN_TEXT = "public.utf8-plain-text",
    FILE_URL = "public.file-url",
    REMOTE_CLIPBOARD = "com.apple.is-remote-clipboard",
}

local HISTORY_TYPE = {
    IMAGE = "IMAGE",
    TEXT = "TEXT",
    FILE = "FILE",
    LARGE_TEXT = "LARGE_TEXT",
}

function selfScriptFileName()
    local str = debug.getinfo(2, "S").source:sub(2)
    return str
end

function getBasename(filePath)
    -- local basename = string.gsub(str, "(.*/)(.*)", "%2")
    local basename = string.match(filePath, "^.+/(.+)$")
    return basename
    
end

function getLastModifyTime(filePath)
    local attr = hs.fs.attributes(filePath)
    if attr == nil then
        return nil
    end
    return attr.modification
end

function genEllipsisTitle(s,i,j)
    i=utf8.offset(s,i)
    j=utf8.offset(s,j+1)
    if i == nil then
        return s
    end
    if j == nil then
        return string.sub(s,i)
    end
    return string.sub(s,i,j) .. "..."
end

function getFileSize(filePath)
    local attr = hs.fs.attributes(filePath)
    if attr == nil then
        return nil
    end
    return attr.size
end

function getFileSha256(filePath)
    local handle = nil
    -- handle = io.popen('/bin/sh -c "sha256sum ' .. filePath .. '"', "r")
    handle = io.popen("shasum -a 256 " .. filePath, "r")
    if handle == nil then
        return nil
    end
    handle:flush()
    local result = handle:read("*all")
    handle:close()
    local hash = result:sub(1, 64)
    return hash
end

function getImageType(uti)
    if uti == UTI_TYPE.IMAGE_TIFF then
        return "tiff"
    elseif uti == UTI_TYPE.IMAGE_PNG then
        return "png"
    elseif uti == UTI_TYPE.IMAGE_JPEG then
        return "jpeg"
    end
end

function removeDirectory(path)
    if hs.fs.attributes(path) == nil then
        return true
    end
    for file in hs.fs.dir(path) do
        if file ~= "." and file ~= ".." then
            local filePath = path .. "/" .. file
            local attr = hs.fs.attributes(filePath)
            if attr.mode == "directory" then
                removeDirectory(filePath)
            else
                local success, err = os.remove(filePath)
                if not success then
                    return false, err
                end
            end
        end
    end

    local success, err = hs.fs.rmdir(path)
    if not success then
        return false, err
    end
    
    return true
end

function toReadableSize(bytes, precision)
    local i = 1
    local units = {"Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"}
    local size = bytes
    while size > 1024 do
        size = size / 1024
        i = i + 1
    end
    local formatted_size = string.format("%." .. precision .. "f %s", size, units[i])
    return formatted_size
end

function readGlobalDatafromDisk(G_default)
    hs.fs.mkdir(DATA_DIR)
    local cacheFile = io.open(G_PATH, "r")
    local G_old = nil
    if cacheFile then
        local content = cacheFile:read("*a")
        if content ~= "" then
            G_old = hs.json.decode(content)
        end
    end

    if G_old and G_old.version == G_default.version then
        return G_old
    else
        removeDirectory(STORAGE_DIR)
        hs.fs.mkdir(STORAGE_DIR)
        saveGlobalDataToDisk(G_default)
        return G_default
    end
end

function saveGlobalDataToDisk(G)
    local cacheFile = io.open(G_PATH, "w")
    cacheFile:write(hs.json.encode(G))
    cacheFile:close()
end

function saveLargeText(content, id)
    local filename = STORAGE_DIR .. string.format("/%08d.txt", id)
    local file = io.open(filename, "w")
    file:write(content)
    file:close()
    return filename
end

function saveImage(image, id, imageType)
    local filename = STORAGE_DIR .. string.format("/%08d.%s", id, imageType)
    local filenameThumb = STORAGE_DIR .. string.format("/%08d.thumb.%s", id, imageType)
    local thumb = image:copy():size({h = 100, w = 100}, false)
    hs.fs.mkdir(STORAGE_DIR)
    image:saveToFile(filename, imageType)
    thumb:saveToFile(filenameThumb, imageType)
    return filename, filenameThumb
end

function readLargeText(filePath)
    local file = io.open(filePath, "r")
    local content = file:read("*a")
    file:close()
    return content
end

function readImage(filePath)
    return hs.image.imageFromPath(filePath)
end

function cleanupLargeTextItem(item)
    os.remove(item.content)
end

function cleanupImageItem(item)
    os.remove(item.content)
    os.remove(item.thumb)
end

function eraseHistoryItem(history, index)
    item = table.remove(history, index)
    if item.type == HISTORY_TYPE.TEXT then
    elseif item.type == HISTORY_TYPE.LARGE_TEXT then
        cleanupLargeTextItem(item)
    elseif item.type == HISTORY_TYPE.IMAGE then
        cleanupImageItem(item)
    elseif item.type == HISTORY_TYPE.FILE then
    end
end

function reduceHistorySize(history, maxSize)
    while #history > maxSize do
        eraseHistoryItem(history, #history)
    end
end

function fromRemoteClipboard(contentTypes)
    for index, uti in ipairs(contentTypes) do
        if uti == UTI_TYPE.REMOTE_CLIPBOARD then
            return true
        end
    end
    return false
end

function addHistoryFromPasteboard(history, id)
    local contentTypes = hs.pasteboard.contentTypes()
    local appname = hs.window.focusedWindow():application():name()
    local datetime = os.date("%Y-%m-%d %H:%M:%S", os.time())
    -- print(appname, hs.inspect(contentTypes))

    local fromRemote = fromRemoteClipboard(contentTypes)
    if fromRemote then
        return
    end

    local item = nil
    for index, uti in ipairs(contentTypes) do
        if uti == UTI_TYPE.IMAGE_TIFF or uti == UTI_TYPE.IMAGE_PNG or uti == UTI_TYPE.IMAGE_JPEG then
            local image = hs.pasteboard.readImage()
            local imageType = getImageType(uti)
            if imageType == "tiff" then
                imageType = "png"
            end
            local imagePath, thumbImagePath = saveImage(image, id, imageType)
            local sha256 = getFileSha256(imagePath)
            local imageSize = image:size()
            item = {
                id = id,
                type = HISTORY_TYPE.IMAGE,
                title = "IMAGE",
                description = datetime, -- .. " / " .. appname,
                content = imagePath,
                thumb = thumbImagePath,
                sha256 = sha256,
                size = { w = imageSize.w, h = imageSize.h }, -- direct assignment will cause json encode error
            }
            break
        elseif uti == UTI_TYPE.FILE_URL then
            local content = hs.pasteboard.readURL()
            local filePath = content.filePath
            local basename = getBasename(filePath)
            local url = content.url
            item = {
                id = id,
                type = HISTORY_TYPE.FILE,
                title = basename,
                description = datetime, -- .. " / " .. appname,
                content = filePath,
                url = url,
            }
            break
        elseif uti == UTI_TYPE.PLAIN_TEXT then
            local content = hs.pasteboard.readString()
            if content == nil then
                return
            end
            local len = string.len(content)
            local u8len = utf8.len(content)
            local title = string.gsub(content, "[\r\n]+", " ")
            title = string.gsub(title, "^%s+", "")
            if u8len < 1 or string.len(title) == 0 then
                return
            end
            title = genEllipsisTitle(title, 1, 100)
            if len >= LARGE_TEXT_THRESHOLD then
                local filePath = saveLargeText(content, id)
                local sha256 = getFileSha256(filePath)
                item = {
                    id = id,
                    type = HISTORY_TYPE.LARGE_TEXT,
                    title = title,
                    description = datetime, -- .. " / " .. appname,
                    content = filePath,
                    u8len = u8len,
                    sha256 = sha256,
                }
            else
                item = {
                    id = id,
                    type = HISTORY_TYPE.TEXT,
                    title = title,
                    description = datetime, -- .. " / " .. appname,
                    content = content,
                    u8len = u8len,
                }
            end
            break
        end
    end
    if item then
        local duplicatePos = -1
        for index, el in ipairs(history) do
            local isImage = item.type == HISTORY_TYPE.IMAGE and el.type == HISTORY_TYPE.IMAGE
            local isText = item.type == HISTORY_TYPE.TEXT and el.type == HISTORY_TYPE.TEXT
            local isLargeText = item.type == HISTORY_TYPE.LARGE_TEXT and el.type == HISTORY_TYPE.LARGE_TEXT
            local isFile = item.type == HISTORY_TYPE.FILE and el.type == HISTORY_TYPE.FILE
            if isText and item.content == el.content then
                duplicatePos = index
                break
            elseif isLargeText and item.sha256 == el.sha256 then
                duplicatePos = index
                break
            elseif isImage and item.sha256 == el.sha256 then
                duplicatePos = index
                break
            elseif isFile and item.content == el.content then
                duplicatePos = index
                break
            end
        end

        if duplicatePos == -1 then
            table.insert(history, 1, item)
            reduceHistorySize(history, MAX_HISTORY_SIZE)
        else
            if item.type == HISTORY_TYPE.TEXT then
            elseif item.type == HISTORY_TYPE.LARGE_TEXT then
                cleanupLargeTextItem(item)
            elseif item.type == HISTORY_TYPE.IMAGE then
                cleanupImageItem(item)
            elseif item.type == HISTORY_TYPE.FILE then
            end
            local oldItem = table.remove(history, duplicatePos)
            table.insert(history, 1, oldItem)
        end
    end    
end

-- https://www.hammerspoon.org/docs/hs.chooser.html#choices
--[[ hs.chooser:choices(choices) -> hs.chooser object
The table of choices (be it provided statically, or returned by the callback) must contain at least the following keys for each choice:
        text - A string or hs.styledtext object that will be shown as the main text of the choice
Each choice may also optionally contain the following keys:
        subText - A string or hs.styledtext object that will be shown underneath the main text of the choice
        image - An hs.image image object that will be displayed next to the choice
        valid - A boolean that defaults to true, if set to false selecting the choice will invoke the invalidCallback method instead of dismissing the chooser
Any other keys/values in each choice table will be retained by the chooser and returned to the completion callback when a choice is made. This is useful for storing UUIDs or other non-user-facing information, however, it is important to note that you should not store userdata objects in the table - it is run through internal conversion functions, so only basic Lua types should be stored.
If a function is given, it will be called once, when the chooser window is displayed. The results are then cached until this method is called again, or hs.chooser:refreshChoicesCallback() is called.
If you're using a hs.styledtext object for text or subText choices, make sure you specify a color, otherwise your text could appear transparent depending on the bgDark setting.
]]

function onShowClipboard(history)
    local choices = {}
    for index, item in ipairs(history) do
        local choice = {}
        if item.type == HISTORY_TYPE.TEXT then
            local readableFileSize = toReadableSize(#item.content, 0)
            choice = {
                text = " " .. item.title,
                subText = " " .. item.description,
                item = item,
            }
            choice.subText = choice.subText .. "  (" .. readableFileSize .. ", " .. item.u8len .. " characters)"
            choice.image = TEXT_ICON
        elseif item.type == HISTORY_TYPE.LARGE_TEXT then
            local fileSize = getFileSize(item.content)
            local readableFileSize = toReadableSize(fileSize, 0)
            choice = {
                text = " " .. item.title,
                subText = " " .. item.description,
                item = item,
            }
            choice.subText = choice.subText .. "  (" .. readableFileSize .. ", " .. item.u8len .. " characters)"
            choice.image = TEXT_ICON
        elseif item.type == HISTORY_TYPE.IMAGE then
            local fileSize = getFileSize(item.content)
            local readableFileSize = toReadableSize(fileSize, 0)
            local uti = hs.fs.fileUTI(item.content)
            local imageType = getImageType(uti)
            choice = {
                text = " " .. item.title,
                subText = " " .. item.description,
                item = item,
            }
            choice.subText = choice.subText .. "  (" .. readableFileSize
            choice.subText = choice.subText .. ", " .. math.ceil(item.size.w) .. "x" .. math.ceil(item.size.h)
            choice.subText = choice.subText .. ", " .. string.upper(imageType)
            choice.subText = choice.subText .. ")"
            choice.image = readImage(item.thumb)
        elseif item.type == HISTORY_TYPE.FILE then
            local fileSize = getFileSize(item.content)
            if fileSize == nil then
                goto continue
            end
            local readableFileSize = toReadableSize(fileSize, 0)
            local uti = hs.fs.fileUTI(item.content)
            local icon = hs.image.iconForFileType(uti)
            choice = {
                text = " FILE: " .. item.title,
                subText = " " .. item.description,
                item = item,
            }
            choice.subText = choice.subText .. "  (" .. readableFileSize .. ", " .. item.content .. ")"
            choice.image = icon
        end
        table.insert(choices, choice)
        ::continue::
    end

    chooser:width(CHOOSER_WIDTH)
    chooser:choices(choices)
    chooser:show()
end

function onChoiceClipboard(choice)
    if choice then
        local item = choice.item
        if item.type == HISTORY_TYPE.TEXT then
            hs.pasteboard.setContents(item.content)
        elseif item.type == HISTORY_TYPE.LARGE_TEXT then
            hs.pasteboard.setContents(readLargeText(item.content))
        elseif item.type == HISTORY_TYPE.IMAGE then
            hs.pasteboard.writeObjects(readImage(item.content))
        elseif item.type == HISTORY_TYPE.FILE then
            hs.pasteboard.writeObjects({ ["url"] = item.url })
        end
        hs.eventtap.keyStroke({ "cmd" }, "v")
    end
    chooser:query("")
    chooser:selectedRow(1)
end

G_default = {
    history = {},
    nextID = 1,
    -- version = "Aug  1 18:05:16 2024",
    version = os.date("%Y-%m-%d %H:%M:%S", getLastModifyTime(selfScriptFileName())),
}

G = readGlobalDatafromDisk(G_default)
chooser = hs.chooser.new(onChoiceClipboard)
preChangeCount = hs.pasteboard.changeCount()
watcher = hs.timer.new(0.5, function()
    local changeCount = hs.pasteboard.changeCount()
    if changeCount ~= preChangeCount then
        G.nextID = G.nextID + 1
        pcall(addHistoryFromPasteboard, G.history, G.nextID)
        pcall(saveGlobalDataToDisk, G)
        -- addHistoryFromPasteboard(G.history, G.nextID)
        -- saveGlobalDataToDisk(G)
        preChangeCount = changeCount
    end
end)
watcher:start()

hs.hotkey.bind({ "cmd", "shift" }, "v", function()
    onShowClipboard(G.history)
end)

local bitmap = {
"data:image/png;base64,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",
"5CYII=",
}
TEXT_ICON = hs.image.imageFromURL(table.concat(bitmap))

-- local fontIcon = hs.image.imageFromPath("/Users/mayeths/Downloads/font-size (9).png")
-- local bitmap = fontIcon:encodeAsURLString("PNG")
-- print(bitmap)
-- TEXT_ICON = hs.image.imageFromURL(bitmap)
-- in python: import textwrap; print(textwrap.fill(s, 80))
