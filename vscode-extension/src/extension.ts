import * as vscode from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
} from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: vscode.ExtensionContext): void {
    const command = process.env['ANAKINS_C_LS_BIN'] ?? 'anakins-c-ls';

    const outputChannel = vscode.window.createOutputChannel('anakins-c-ls');
    outputChannel.appendLine(`Starting anakins-c-ls: ${command}`);
    outputChannel.show();

    const serverOptions: ServerOptions = { command };
    const clientOptions: LanguageClientOptions = {
        documentSelector: [
            { scheme: 'file', language: 'c' },
            { scheme: 'file', language: 'cpp' },
        ],
        outputChannel,
    };
    client = new LanguageClient(
        'anakins-c-ls',
        'C/C++ Language Server',
        serverOptions,
        clientOptions,
    );

    client.start().catch((err: unknown) => {
        outputChannel.appendLine(`Failed to start: ${err}`);
        vscode.window.showErrorMessage(`anakins-c-ls failed to start: ${err}`);
    });

    context.subscriptions.push({ dispose: () => client?.stop() });
}

export function deactivate(): Thenable<void> | undefined {
    return client?.stop();
}
