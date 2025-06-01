hs.console.clearConsole()
require("modules.clipboard")
require("modules.test")
print("-----------------------------------------")
print(">>>> Hammerspoon config load success <<<<")
print("-----------------------------------------")
hs.alert.show("Hammerspoon config load success")

function reloadConfigCallback(files)
  doReload = false
  for _, file in pairs(files) do
    if file:sub(-4) == ".lua" then
      doReload = true
    end
  end
  if doReload then
    hs.reload()
  end
end

local configPath = os.getenv("HOME") .. "/.hammerspoon/"
reloadWatcher = hs.pathwatcher.new(configPath, reloadConfigCallback):start()
