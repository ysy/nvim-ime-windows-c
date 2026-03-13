# nvim-ime-windows-c

Windows 下用 Neovim 最大的痛点之一：在 Insert 模式用中文（五笔/拼音）写完注释，按下 `Esc` 回到 Normal 模式，或者从终端其他 Tab 切回 Neovim 时，输入法还是中文状态。一敲快捷键，全是乱码上屏。

市面上常见的 `im-select` 方案通常需要在 Windows 里强行加一个美式键盘布局 (1033)。如果你只想保留纯粹的单一中文输入法，且需要一个**零依赖、极低资源占用**的方案，这个纯 C 小工具刚好适合你。

它只做一件事：向当前窗口发送底层 `WM_IME_CONTROL` 消息，强制把输入法状态拨回“英文”，绝不误杀。

## 1. 编译

源码极简，用 `WinMain` 编写，后台静默运行无黑框闪烁。
确保系统有 GCC（如 MinGW），直接在终端执行：

```bash
gcc set_ime_en.c -o set_ime_en.exe -limm32 -mwindows -O2
```



## 2. nvim 配置
```lua
local set_ime_en_cmd = "set_ime_en.exe" 

vim.api.nvim_create_autocmd({"FocusGained", "InsertLeave"}, {
  pattern = "*",
  callback = function()
    local mode = vim.api.nvim_get_mode().mode
    
    -- 只在 Normal 或 Visual 模式下强制切英文
    if mode == 'n' or mode:match('^[vV\x16]') then
      -- 异步静默执行，不卡主线程
      vim.fn.jobstart({set_ime_en_cmd}, {detach = true})
    end
  end,
})
```


## 3. 不依赖set_ime_en.exe的方法，纯lua实现
```lua
-- ====== Windows 下利用 LuaJIT FFI 极速切换输入法 ======

local ffi = require("ffi")

-- 1. 声明需要用到的 Windows C API 和类型
ffi.cdef[[
    typedef void* HWND;
    typedef unsigned int UINT;
    typedef long LRESULT;
    
    HWND GetForegroundWindow();
    HWND ImmGetDefaultIMEWnd(HWND);
    LRESULT SendMessageA(HWND, UINT, UINT, LRESULT);
]]

-- 2. 加载 Windows 的 user32 和 imm32 动态链接库
local user32 = ffi.load("user32.dll")
local imm32 = ffi.load("imm32.dll")

-- 3. 封装底层调用函数
local function set_ime_en_fast()
    local hwnd = user32.GetForegroundWindow()
    if hwnd ~= nil then
        local imeHwnd = imm32.ImmGetDefaultIMEWnd(hwnd)
        if imeHwnd ~= nil then
            -- 0x0283: WM_IME_CONTROL
            -- 0x0006: IMC_SETOPENSTATUS
            -- 0: 英文状态 (关闭中文)
            user32.SendMessageA(imeHwnd, 0x0283, 0x0006, 0)
        end
    end
end

-- 4. 绑定到 Neovim 事件
vim.api.nvim_create_autocmd({"FocusGained", "InsertLeave"}, {
  pattern = "*",
  callback = function()
    local mode = vim.api.nvim_get_mode().mode
    if mode == 'n' or mode:match('^[vV\x16]') then
        -- 直接在内存中极速调用，不需要 jobstart
        set_ime_en_fast()
    end
  end,
})
```

## 参考项目
[im-select.nvim](https://github.com/keaising/im-select.nvim)
