#include "Renderer.hpp"
#include "TextLayout.hpp"

namespace sjtu {

//我们在Render中保留了所有需要用到控制序列的部分逻辑,你不应该修改它们.
//啥是控制序列?它们是嵌入在文本流中的“指令”,告诉终端执行特定操作(比如移动光标、改变文字颜色、清屏或清除行内容).   
namespace {

void AppendClearedLine(std::string& frame, std::string_view contents, std::size_t width, bool newline) {
    //追加不超过width列的显示内容,再用ESC[K清除这一行余下的旧内容
    //需要换行时同时输出回车和换行,因为raw模式关闭了终端的自动输出转换
    //你在生成帧的时候生成每一行都应该调用这个函数
    frame.append(contents.substr(0, width));
    frame += "\x1b[K";
    if (newline) {
        frame += "\r\n";
    } 
}

std::string CursorSequence(std::size_t row, std::size_t column) {
    //生成定位光标的ANSI转义序列,这里接收的行列都从1开始
    return "\x1b[" + std::to_string(row) + ';' + std::to_string(column) + 'H';
}

}

std::string Renderer::Render(const Buffer& buffer, const Window& window, const RenderState& state) const {
    //1. 隐藏光标并回到屏幕左上角,开始拼接新的一帧(一帧就是一个长string)
    //2. 按视口逐行取Buffer内容,展开Tab后再横向截取;超出文件的行显示~
    //3. 绘制底部一行,优先显示命令行,其次是提示信息,最后是Insert模式标记
    //4. 根据当前模式计算光标的屏幕位置,追加定位序列并重新显示光标
    //你应该区分我们的Cursor光标(它代表当前用户在修改文件的哪个位置)和终端显示的白色亮条,白色亮条在屏幕中的位置是由光标位置和视口位置计算的
    auto& viewport = window.GetViewport();
    auto width = std::max<std::size_t>(viewport.columns_, 1);

    std::string frame;
    frame.reserve((viewport.rows_ + 1) * (width + 8));
    frame += "\x1b[?25l";
    frame += "\x1b[H";

    for (std::size_t screen_row = 0; screen_row < viewport.rows_; ++screen_row) 
    {
        frame += "\x1b[K";
        if(screen_row+viewport.top_>=buffer.GetLineCount())
        {
            frame+="\x1b[34m";
            AppendClearedLine(frame,"~",width,false);
            frame+="\x1b[39m\r\n";
        }
        else
        {
            const std::string& line=buffer.GetLineAt(screen_row+viewport.top_);
            size_t l=RenderColumnToBufferColumn(line,viewport.left_);
            size_t r=RenderColumnToBufferColumn(line,viewport.left_+viewport.columns_+(size_t)1);
            if(viewport.left_>=line.size())
            {
                AppendClearedLine(frame,"",width,true);
                continue;
            }
            AppendClearedLine(frame,line.substr(l,r-l),width,true);
        }
    }

    std::string bottom;
    if (state.mode_ == Mode::CommandLine) {
        bottom = ":" + state.command_;
    } else if (!state.message_.empty()) {
        bottom = state.message_;
        if(state.message_.substr(0,6)=="Error:")
        {
            frame+="\x1b[37m";
            frame+="\x1b[41m";
        }
    } else if (state.mode_ == Mode::Insert) {
        bottom = "-- INSERT --";
    }
    AppendClearedLine(frame, bottom, width, false);
    frame+="\x1b[0m";
    //这里是提示3中的部分
    //我们只会在CommandMode的时候检查一下底部的命令内容,在NormalMode不会看底部,所以message你可以随意写

    std::size_t cursor_row{0};
    std::size_t cursor_column{0};

    //计算cursor_row和cursor_column即可    
    if(state.mode_ == Mode::CommandLine)
    {
        cursor_row=viewport.rows_;
        cursor_column=1+BufferColumnToRenderColumn(state.command_,state.command_cursor);
    }
    else
    {
        cursor_row=window.GetCursor().row_-viewport.top_;
        cursor_column=BufferColumnToRenderColumn(buffer.GetLineAt(window.GetCursor().row_),window.GetCursor().column_),-viewport.left_;
    }
    frame += CursorSequence(cursor_row+1, cursor_column+1);
    frame += "\x1b[?25h";
    return frame;
}

std::string Renderer::ExpandForDisplay(std::string_view line) {
    //从左到右扫描buffer中一整行的实际内容,并扩展到render应该输出的视图
    //你应该在Render中调用这个函数,并把函数返回的结果按照视口剪切用于Render的某些行
    //我似乎暂时可以不用到，先观望一下
    return {};
}
} // namespace sjtu
