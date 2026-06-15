const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

function activate(context) {
    const disposable = vscode.commands.registerCommand('stergosmart.configure', () => {
        const panel = vscode.window.createWebviewPanel(
            'stergoSmartConfig',
            'StergoSmart Config',
            vscode.ViewColumn.One,
            { enableScripts: true }
        );

        // Load current settings.h values
        const workspaceFolders = vscode.workspace.workspaceFolders;
        let current = {
            program: "0",
            screen: "0",
            board: "1",
            plug: "1",
            debug: "0"
        };

        if (workspaceFolders && workspaceFolders.length > 0) {
            const file = path.join(workspaceFolders[0].uri.fsPath, 'settings.h');

            if (fs.existsSync(file)) {
                const text = fs.readFileSync(file, 'utf8');

                const get = (name, fallback) => {
                    const m = text.match(new RegExp(`#define\\s+${name}\\s+(\\S+)`));
                    return m ? m[1] : fallback;
                };

                current.program = get("STERGO_PROGRAM", "0");
                current.screen  = get("STERGO_SCREEN", "0");
                current.board   = get("STERGO_PROGRAM_BOARD", "1");
                current.plug    = get("STERGO_PLUG", "1");
                current.debug   = get("DEBUG", "0");
            }
        }

        // Load HTML
        const htmlPath = vscode.Uri.joinPath(context.extensionUri, 'media', 'ui.html');

        fs.readFile(htmlPath.fsPath, 'utf8', (err, data) => {
            if (err) {
                vscode.window.showErrorMessage('Failed to load UI.');
                return;
            }

            const cssUri = panel.webview.asWebviewUri(
                vscode.Uri.joinPath(context.extensionUri, 'media', 'style.css')
            );

            panel.webview.html = data
                .replace('{{cssUri}}', cssUri.toString())
                .replace('{{PROGRAM}}', current.program)
                .replace('{{SCREEN}}', current.screen)
                .replace('{{BOARD}}', current.board)
                .replace('{{PLUG}}', current.plug)
                .replace('{{DEBUG}}', current.debug);
        });

        // Save handler
        panel.webview.onDidReceiveMessage(msg => {
            const workspaceFolders = vscode.workspace.workspaceFolders;
            if (!workspaceFolders || workspaceFolders.length === 0) {
                vscode.window.showErrorMessage('No workspace folder open.');
                return;
            }

            const file = path.join(workspaceFolders[0].uri.fsPath, 'settings.h');

            if (!fs.existsSync(file)) {
                vscode.window.showErrorMessage('settings.h not found in workspace root.');
                return;
            }

            let text = fs.readFileSync(file, 'utf8');

            text = text.replace(/#define\s+STERGO_PROGRAM\s+.*/, `#define STERGO_PROGRAM ${msg.program}`);
            text = text.replace(/#define\s+STERGO_SCREEN\s+.*/, `#define STERGO_SCREEN ${msg.screen}`);
            text = text.replace(/#define\s+STERGO_PROGRAM_BOARD\s+.*/, `#define STERGO_PROGRAM_BOARD ${msg.board}`);
            text = text.replace(/#define\s+STERGO_PLUG\s+.*/, `#define STERGO_PLUG ${msg.plug}`);
            text = text.replace(/#define\s+DEBUG\s+.*/, `#define DEBUG ${msg.debug}`);

            fs.writeFileSync(file, text, 'utf8');
            vscode.window.showInformationMessage('settings.h updated');
        });
    });

    context.subscriptions.push(disposable);
}

function deactivate() {}

module.exports = {
    activate,
    deactivate
};
