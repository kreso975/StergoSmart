const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

function activate(context) {
    const output = vscode.window.createOutputChannel("StergoSmart Debug");
    output.show(true);

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
            debug: "0",
            fw_version: "",
            model_name: "",
            company_url: "",
            led_pin: "0",
            matrix_orientation: "1",
            gpio_sda: "0",
            gpio_scl: "0",
            dht_pin: "0",
            dht_type: ""
        };

        if (workspaceFolders && workspaceFolders.length > 0) {
            const file = path.join(workspaceFolders[0].uri.fsPath, 'settings.h');

            if (fs.existsSync(file)) {
                const text = fs.readFileSync(file, 'utf8');

                const get = (name, fallback) => {
                    const regex = new RegExp(`^\\s*#define\\s+${name}\\s+(?:"([^"]+)"|(\\S+))`, "m");
                    const m = text.match(regex);

                    const value = m ? (m[1] || m[2]) : fallback;

                    return (value === undefined || value === null) ? fallback : value;
                };

                current.program = get("STERGO_PROGRAM", "0");
                current.screen  = get("STERGO_SCREEN", "0");
                current.board   = get("STERGO_PROGRAM_BOARD", "1");
                current.plug    = get("STERGO_PLUG", "1");
                current.debug   = get("DEBUG", "0");
                current.fw_version = get("FW_VERSION", "\"000.00.000\"").replace(/"/g, "");
                current.model_name = get("MODEL_FRENDLY_NAME", "\"Stergo Smart\"").replace(/"/g, "");
                current.company_url = get("COMPANY_URL", "\"http://www.stergo.hr\"").replace(/"/g, "");
                current.led_pin = get("LED_PIN", "0");
                current.matrix_orientation = get("MATRIX_ORIENTATION", "1");
                current.gpio_sda = get("GPIO_SDA", "0");
                current.gpio_scl = get("GPIO_SCL", "0");
                current.dht_pin = get("DHTPIN", "0");
                current.dht_type = get("DHTTYPE", "\"DHT22\"").replace(/"/g, "");
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
                .replace('{{DEBUG}}', current.debug)
                .replace('{{FW_VERSION}}', current.fw_version)
                .replace('{{MODEL_NAME}}', current.model_name)
                .replace('{{COMPANY_URL}}', current.company_url)
                .replace('{{LED_PIN}}', current.led_pin)
                .replace('{{MATRIX_ORIENTATION}}', current.matrix_orientation)
                .replace('{{GPIO_SDA}}', current.gpio_sda)
                .replace('{{GPIO_SCL}}', current.gpio_scl)
                .replace('{{DHTPIN}}', current.dht_pin)
                .replace('{{DHTTYPE}}', current.dht_type);
        });

        // Save handler
        panel.webview.onDidReceiveMessage(msg => {

            // 1. Handle reload request FIRST
            if (msg.command === "reloadPanel") {
                panel.dispose();
                vscode.commands.executeCommand('stergosmart.configure');
                return;
            }

            // 2. Handle SAVE
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
            text = text.replace(/#define\s+LED_PIN\s+.*/, `#define LED_PIN ${msg.led_pin}`);
            text = text.replace(/#define\s+MATRIX_ORIENTATION\s+.*/, `#define MATRIX_ORIENTATION ${msg.matrix_orientation}`);
            text = text.replace(/#define\s+GPIO_SDA\s+.*/, `#define GPIO_SDA ${msg.gpio_sda}`);
            text = text.replace(/#define\s+GPIO_SCL\s+.*/, `#define GPIO_SCL ${msg.gpio_scl}`);
            text = text.replace(/#define\s+DHTPIN\s+.*/, `#define DHTPIN ${msg.dht_pin}`);
            text = text.replace(/#define\s+DHTTYPE\s+.*/, `#define DHTTYPE "${msg.dht_type}"`);

            text = text.replace(/#define\s+FW_VERSION\s+.*/, `#define FW_VERSION "${msg.fw_version}"`);
            text = text.replace(/#define\s+MODEL_FRENDLY_NAME\s+.*/, `#define MODEL_FRENDLY_NAME "${msg.model_name}"`);
            text = text.replace(/#define\s+COMPANY_URL\s+.*/, `#define COMPANY_URL "${msg.company_url}"`);

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
