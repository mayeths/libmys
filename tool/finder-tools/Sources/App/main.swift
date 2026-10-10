import AppKit

final class AppDelegate: NSObject, NSApplicationDelegate {
    func applicationDidFinishLaunching(_ notification: Notification) {
        NSApp.setActivationPolicy(.accessory)
        NSApp.activate(ignoringOtherApps: true)

        let alert = NSAlert()
        alert.messageText = "FinderTools is installed"
        alert.informativeText = """
        Enable FinderToolsExtension in:

        System Settings > General > Login Items & Extensions > Finder Extensions

        Then relaunch Finder if the context menu does not appear.
        """
        alert.addButton(withTitle: "OK")
        alert.runModal()

        NSApp.terminate(nil)
    }
}

let application = NSApplication.shared
let applicationDelegate = AppDelegate()
application.delegate = applicationDelegate
application.run()
