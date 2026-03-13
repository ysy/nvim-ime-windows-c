#include <windows.h>
#include <imm.h>

// 使用 WinMain 而不是 main，这样编译出来的程序在后台静默运行，不会闪烁控制台黑框
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 获取当前处于活动状态的窗口（即你切回来的 Neovim 所在的 Windows Terminal）
    HWND hwnd = GetForegroundWindow();
    
    if (hwnd != NULL) {
        // 获取该窗口绑定的默认输入法(IME)窗口句柄
        HWND imeHwnd = ImmGetDefaultIMEWnd(hwnd);
        
        if (imeHwnd != NULL) {
            // 发送 Windows 消息，强制设置输入法状态
            // 0x0283 = WM_IME_CONTROL (控制输入法的消息)
            // 0x0006 = IMC_SETOPENSTATUS (设置输入法开启/关闭状态)
            // 0 = 关闭状态 (即英文状态)
            SendMessage(imeHwnd, 0x0283, 0x0006, 0);
        }
    }
    
    return 0;
}
