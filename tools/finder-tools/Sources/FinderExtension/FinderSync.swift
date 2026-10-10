import AppKit
import FinderSync

final class FinderSync: FIFinderSync {
    private let controller = FIFinderSyncController.default()

    override init() {
        super.init()
        controller.directoryURLs = [URL(fileURLWithPath: "/", isDirectory: true)]
    }

    override func menu(for menuKind: FIMenuKind) -> NSMenu? {
        switch menuKind {
        case .contextualMenuForItems, .contextualMenuForSidebar:
            guard let urls = controller.selectedItemURLs(), !urls.isEmpty else {
                return nil
            }

            let title = urls.count == 1 ? "Copy Path" : "Copy Paths"
            return makeMenu(title: title, action: #selector(copySelectedPaths(_:)))

        case .contextualMenuForContainer:
            guard controller.targetedURL() != nil else {
                return nil
            }

            return makeMenu(title: "Copy Folder Path", action: #selector(copyTargetedFolderPath(_:)))

        default:
            return nil
        }
    }

    private func makeMenu(title: String, action: Selector) -> NSMenu {
        let menu = NSMenu()
        let item = NSMenuItem(title: title, action: action, keyEquivalent: "")
        item.target = self
        menu.addItem(item)
        return menu
    }

    @objc private func copySelectedPaths(_ sender: Any?) {
        guard let urls = controller.selectedItemURLs(), !urls.isEmpty else {
            return
        }

        copyPaths(from: urls)
    }

    @objc private func copyTargetedFolderPath(_ sender: Any?) {
        guard let url = controller.targetedURL() else {
            return
        }

        copyPaths(from: [url])
    }

    private func copyPaths(from urls: [URL]) {
        let paths = urls.map(\.path).joined(separator: "\n")
        let pasteboard = NSPasteboard.general
        pasteboard.clearContents()
        pasteboard.setString(paths, forType: .string)
    }
}
