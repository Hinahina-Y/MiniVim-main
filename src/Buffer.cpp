#include <fstream>

#include "Buffer.hpp"

namespace sjtu
{

    Buffer::Buffer(const std::filesystem::path &path)
    {
        lines_.clear();
        if (path.empty())
        {
            lines_.push_back("");
            return;
        }
        std::string line;
        std::ifstream in(path); // 读取文件，由ai建议我这么写
        while (std::getline(in, line))
        {
            if (!line.empty() && line.back() == '\r') // 防止Windows文件带换行\r\n
            {
                line.pop_back();
            }
            lines_.push_back(line);
        }
        // 从path指向的文件构造Buffer,你需要打开文件并且把文件内容填充进Buffer,并正确初始化一些状态.
        // 注意path可能为空的边界情况
    }

    Buffer::Buffer(std::vector<std::string> lines, std::filesystem::path path)
    {
        throw std::runtime_error("Not implemented.");
    }

    std::size_t Buffer::GetLineCount() const
    {
        // 返回文件行数
        return lines_.size();
    }

    const std::string &Buffer::GetLineAt(std::size_t row) const
    {
        // 返回第row行的内容
        return lines_[row];
    }

    std::string Buffer::GetDisplayName() const
    {
        // 返回文件名,若是新文件,返回"[No Name]"
        if (path_.empty())
        {
            return "[No Name]";
        }
        return path_.filename().string();
    }

    bool Buffer::IsModified() const
    {
        // 返回文件和上次保存比起来是否被修改过
        if (path_.empty())
        {
            return true;
        }
        std::string line;
        std::ifstream in(path_); // 读取文件，由ai建议我这么写
        int lines_index = 0;
        while (std::getline(in, line))
        {
            if (!line.empty() && line.back() == '\r') // 防止Windows文件带换行\r\n
            {
                line.pop_back();
            }
            if (lines_index >= lines_.size() || lines_[lines_index] != line)
            {
                return true;
            }
            lines_index++;
        }
        if (lines_index != lines_.size())
        {
            return true;
        }
        return false;
    }

    void Buffer::InsertCharacter(std::size_t row, std::size_t column, char value)
    {
        if (row >= lines_.size() || column > lines_[row].size() + 1)
        {
            // 越界,暂时不知道如何处理
        }
        lines_[row].insert(lines_[row].begin() + column, value);
        // 在第row行第col列插入一个value, 注意越界检查
        return;
    }

    void Buffer::EraseCharacter(std::size_t row, std::size_t column)
    {
        if (row >= lines_.size() || column > lines_[row].size())
        {
            throw std::runtime_error("越界了！");
            return;
            // 越界,暂时不知道如何处理
        }
        lines_[row].erase(column - 1, 1);
    }

    void Buffer::SplitLine(std::size_t row, std::size_t column)
    {
        // 在第row行第col列分割,即在此处敲了回车键
        std::string line_new = lines_[row].substr(column);
        lines_[row] = lines_[row].substr(0, column);
        lines_.insert(lines_.begin() + row + 1, line_new);
    }

    void Buffer::JoinLine(std::size_t row)
    {
        // 把第row + 1行合并进第row行
        lines_[row] += lines_[row + 1];
        lines_.erase(lines_.begin() + row + 1);
    }

    void Buffer::Save()
    {
        // 把文件内容保存, 直接调用WriteTo方法
        WriteTo(path_);
    }

    void Buffer::SaveAs(const std::filesystem::path &path)
    {
        path_ = path;
        WriteTo(path);
    }

    void Buffer::WriteTo(const std::filesystem::path &path) const
    {
        // 实际将缓冲区中的内容写入path指向的文件中
        if (path.empty())
        {
            throw std::runtime_error("Error:No File Name.");
            return;
        }

        std::ofstream out(path); // 写入文件，由ai建议我这么写
        if (!out)
        {
            throw std::runtime_error("Error:Failed to open file.");
            return;
        }

        for (const auto &line : lines_)
        {
            out << line << '\r\n';
        }

        if (!out)
        {
            throw std::runtime_error("Error:File Error.");
        }
        return;
    }

} // namespace sjtu
