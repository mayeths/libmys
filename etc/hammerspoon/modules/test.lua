-- local chooser = require("hs.chooser")
-- local console = require("hs.console")
-- local inspect = require("hs.inspect")
-- local styledtext = require("hs.styledtext")

-- console.clearConsole()

-- local completionFn = function(result)
--     print(string.format("Result: %s", result and inspect(result)))	
    
--     print("text:")
--     console.printStyledtext(result.text)
--     print("subText:")
--     console.printStyledtext(result.subText)

-- end

-- local text = styledtext.new("Styled Text", {
--         color = {hex = "#FF0000", alpha = 1},
--         font = { name = "Futura", size = 18 },
--     })

-- local subText = styledtext.new("Styled Subtext", {
--         color = {hex = "#800000", alpha = 1},
--         font = { name = "Futura", size = 10 },
--     })

-- print("text:")
-- console.printStyledtext(text)
-- print("subText:")
-- console.printStyledtext(subText)

-- local choices = {}

-- for i, v in pairs(styledtext.fontNames()) do
--     local choice = {}	
--     if string.sub(v, 1, 1) ~= "." then 
--         local displayName = styledtext.fontInfo(v).displayName
--         choice.text = styledtext.new(displayName, {
--             font = { name = v, size = 18 },
--         })
--         --choice.subText = ""
--         choice.uuid = choice.text
--         table.insert(choices, choice)
--     end		
-- end

-- theChooser = chooser.new(completionFn)
--     :choices(choices)
--     :show()

-- print("asdfjkslf")
-- t = require("hs.webview.toolbar")
-- a = t.new("myConsole", {
--         { id = "select1", selectable = true, image = hs.image.imageFromName("NSStatusAvailable") },
--         { id = "NSToolbarSpaceItem" },
--         { id = "select2", selectable = true, image = hs.image.imageFromName("NSStatusUnavailable") },
--         { id = "notShown", default = false, image = hs.image.imageFromName("NSBonjour") },
--         { id = "NSToolbarFlexibleSpaceItem" },
--         { id = "navGroup", label = "Navigation", groupMembers = { "navLeft", "navRight" }},
--         { id = "navLeft", image = hs.image.imageFromName("NSGoLeftTemplate"), allowedAlone = false },
--         { id = "navRight", image = hs.image.imageFromName("NSGoRightTemplate"), allowedAlone = false },
--         { id = "NSToolbarFlexibleSpaceItem" },
--         { id = "cust", label = "customize", fn = function(t, w, i) t:customizePanel() end, image = hs.image.imageFromName("NSAdvanced") }
--     }):canCustomize(true)
--       :autosaves(true)
--       :selectedItem("select2")
--       :setCallback(function(...)
--                         print("a", inspect(table.pack(...)))
--                    end)

-- t.attachToolbar(a)




-- t = require("hs.webview.toolbar")
-- function testToolbar(chooser)
--     a = t.new("myConsoleToolbar", {
--             { id = "local", label = "Macbook", selectable = true, image = hs.image.imageFromName("NSStatusAvailable") },
--             -- { id = "NSToolbarSpaceItem" },
--             { id = "remote", label = "iPhone/iPad", selectable = true, image = hs.image.imageFromName("NSStatusUnavailable") },
--             { id = "all", label = "All", selectable = true, image = hs.image.imageFromName("NSStatusUnavailable") },
--             { id = "NSToolbarFlexibleSpaceItem" },
--         })
--         :canCustomize(true)
--         :selectedItem("local")
--         :setCallback(function(...)
--             print("a", hs.inspect(table.pack(...)))
--             local win = hs.window.focusedWindow()
--             local title = win:title()
--             print("Old title: ", title)
--             local axWindow = hs.axuielement.windowElement(win)
--             if not axWindow then
--                 print("Failed to get AXUIElement of the window.")
--                 return
--             end
--             local newTitle = "HAHHAHA"
--             local success = axWindow:setAttributeValue("AXTitle", newTitle)
--             if success then
--                 print("Window title changed to: " .. newTitle)
--             else
--                 print("Failed to change window title.")
--             end
--         end)
--     a:toolbarStyle("automatic")
--     a:displayMode("default")
--     a:sizeMode("small")

--     t.attachToolbar(chooser, a)
-- end

-- testToolbar(chooser)
