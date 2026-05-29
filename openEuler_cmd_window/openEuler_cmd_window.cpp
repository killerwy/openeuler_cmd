#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <iomanip>
#include "openEuler_cmd_core.h"
using namespace std;
vector<string> split_cmd(const string& cmd)// 拆分命令行，按空格分割命令和参数
{
    vector<string> parts;
    string part;
    for (char c : cmd)
    {
        if (c == ' ')
        {
            if (!part.empty())
            {
                parts.push_back(part);
                part.clear();
            }
        }
        else
            part += c;
    }
    if (!part.empty())
        parts.push_back(part);
    return parts;
}
string format_time(FILETIME ft)// 格式化FILETIME
{
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    char buf[64];
    sprintf_s(buf, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return buf;
}
int main()
{
    char current_dir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, current_dir);
    cout << "==================================================" << endl;
    cout << "  Windows兼容openEuler文件系统命令接口" << endl;
    cout << "  支持命令：ls、cat、cp、rm、mv、mkdir、rmdir、cd、find、help、exit" << endl;
    cout << "==================================================" << endl << endl;
    while (true)// 命令循环
    {
        cout << "[openEuler@Windows " << current_dir << "]$ ";
        string cmd;
        getline(cin, cmd);
        vector<string> parts = split_cmd(cmd);
        if (parts.empty())
            continue;
        string cmd_name = parts[0];
        if (cmd_name == "exit")// 退出
        {
            cout << "退出" << endl;
            break;
        }
        else if (cmd_name == "ls")
        {
            BOOL show_all = FALSE, long_format = FALSE, human_readable = FALSE;
            const char* path = NULL;
            for (size_t i = 1; i < parts.size(); i++)
            {
                const string& part = parts[i];
                if (part[0] == '-')
                    for (size_t j = 1; j < part.size(); j++)
                        switch (part[j])
                        {
                        case 'a':
                            show_all = TRUE;
                            break;
                        case 'l':
                            long_format = TRUE;
                            break;
                        case 'h':
                            human_readable = TRUE;
                            break;
                        default:
                            cout << "错误：未知选项 '" << part[j] << "'" << endl;
                        }
                else
                    if (path == NULL)
                        path = part.c_str();
                    else
                        cout << "错误：多余参数 '" << part << "'" << endl;
            }
            DirEntry* entries = NULL;
            int count = 0;
            char err_msg[256];
            int ret = ls(path, &entries, &count, err_msg, show_all, long_format, human_readable);
            if (ret != CMD_SUCCESS)
                cout << "错误：" << err_msg << endl;
            else
            {
                cout << "目录内容（共" << count << "项）：" << endl;
                if (long_format)
                {
                    cout << left << setw(40) << "名称" << setw(10) << "类型" << setw(12) << "大小(字节)" << "修改时间" << endl;
                    cout << "---------------------------------------------------------------------------------" << endl;
                    for (int i = 0; i < count; i++)
                        if (human_readable)
                            cout << left << setw(40) << entries[i].filename << setw(10) << (entries[i].is_directory ? "目录" : "文件") << setw(12) << entries[i].hfile_size << format_time(entries[i].modify_time) << endl;
                        else
                            cout << left << setw(40) << entries[i].filename << setw(10) << (entries[i].is_directory ? "目录" : "文件") << setw(12) << entries[i].file_size << format_time(entries[i].modify_time) << endl;
                }
                else
                    for (int i = 0; i < count; i++)
                        cout << entries[i].filename << endl;
            }
            if (entries)
                free(entries);
        }
        else if (cmd_name == "cat")
        {
            BOOL show_number = FALSE, show_number_nonblank = FALSE, squeeze_blank = FALSE;
            string filename;
            for (size_t i = 1; i < parts.size(); i++)
            {
                const string& part = parts[i];
                if (part[0] == '-')
                    for (size_t j = 1; j < part.size(); j++)
                        switch (part[j])
                        {
                        case 'n':
                            show_number = TRUE;
                            break;
                        case 'b':
                            show_number_nonblank = TRUE;
                            break;
                        case 's':
                            squeeze_blank = TRUE;
                            break;
                        default:
                            cout << "错误：未知选项 '" << part[j] << "'" << endl;
                        }
                else
                    if (filename.empty())
                        filename = part;
                    else
                        cout << "错误：多余参数 '" << part << "'" << endl;
            }
            if (filename.empty())
                cout << "错误：缺少参数" << endl;
            char buffer[4096];
            char err_msg[256];
            int ret = cat(filename.c_str(), buffer, sizeof(buffer), err_msg, show_number, show_number_nonblank, squeeze_blank);
            if (ret != CMD_SUCCESS)
                cout << "错误：" << err_msg << endl;
            else
                cout << "文件内容：" << endl << buffer << endl;
        }
        else if (cmd_name == "cp")
        {
            BOOL recursive = FALSE;
            vector<string> paths;
            for (size_t i = 1; i < parts.size(); i++)
            {
                const string& part = parts[i];

                if (part == "-r" || part == "-R")
                    recursive = TRUE;
                else
                    paths.push_back(part);
            }
            if (paths.size() != 2)
            {
                cout << "错误：参数错误" << endl;
                continue;
            }
            char err_msg[256];
            int ret = cp(paths[0].c_str(), paths[1].c_str(), err_msg, recursive);
            ret == CMD_SUCCESS ? cout << "复制成功！" << endl : cout << "错误：" << err_msg << endl;
        }
        else if (cmd_name == "rm")
        {
            BOOL recursive = FALSE;
            vector<string> paths;
            for (size_t i = 1; i < parts.size(); i++)
            {
                const string& part = parts[i];
                if (part == "-r" || part == "-R")
                    recursive = TRUE;
                else
                    paths.push_back(part);
            }
            if (paths.size() != 1)
            {
                cout << "错误：参数错误" << endl;
                continue;
            }
            char err_msg[256];
            int ret = rm(paths[0].c_str(), err_msg, recursive);
            ret == CMD_SUCCESS ? cout << "删除成功！" << endl : cout << "错误：" << err_msg << endl;
        }
        else if (cmd_name == "mv")
        {
            if (parts.size() < 3)
            {
                cout << "错误：缺少参数" << endl;
                continue;
            }
            char err_msg[256];
            int ret = mv(parts[1].c_str(), parts[2].c_str(), err_msg);
            ret == CMD_SUCCESS ? cout << "移动成功！" << endl : cout << "错误：" << err_msg << endl;
        }
        else if (cmd_name == "mkdir")
        {
            BOOL create_parent = FALSE;
            string dir_path;
            for (size_t i = 1; i < parts.size(); i++)
            {
                const string& part = parts[i];
                if (part == "-p")
                    create_parent = TRUE;
                else
                    if (dir_path.empty())
                        dir_path = part;
                    else
                        cout << "错误：多余参数 '" << part << "'" << endl;
            }
            if (dir_path.empty())
            {
                cout << "错误：缺少参数" << endl;
                continue;
            }
            char err_msg[256];
            int ret = mkdir(dir_path.c_str(), err_msg, create_parent);
            ret == CMD_SUCCESS ? cout << "目录创建成功！" << endl : cout << "错误：" << err_msg << endl;
        }
        else if (cmd_name == "rmdir")
        {
            BOOL remove_parent = FALSE;
            string dir_path;
            for (size_t i = 1; i < parts.size(); i++)
            {
                const string& part = parts[i];
                if (part == "-p")
                    remove_parent = TRUE;
                else
                    if (dir_path.empty())
                        dir_path = part;
                    else
                        cout << "错误：多余参数 '" << part << "'" << endl;
            }
            if (dir_path.empty())
            {
                cout << "错误：缺少参数" << endl;
                continue;
            }
            char err_msg[256];
            BOOL* first_rm = new BOOL;
            *first_rm = TRUE;
            int ret = rmdir(dir_path.c_str(), err_msg, remove_parent, first_rm);
            (ret == CMD_SUCCESS || *first_rm == FALSE) ? cout << "目录删除成功！" << endl : cout << "错误：" << err_msg << endl;
        }
        else if (cmd_name == "cd")
        {
            if (parts.size() < 2)
            {
                cout << "错误：缺少参数" << endl;
                continue;
            }
            char err_msg[256];
            int ret = cd(parts[1].c_str(), current_dir, MAX_PATH, err_msg);
            if (ret != CMD_SUCCESS) cout << "错误：" << err_msg << endl;
        }
        else if (cmd_name == "find")
        {
            if (parts.size() < 3)
            {
                cout << "错误：缺少参数" << endl;
                continue;
            }
            char result[8192];
            char err_msg[256];
            int ret = find(parts[1].c_str(), parts[2].c_str(), result, sizeof(result), err_msg);
            if (ret != CMD_SUCCESS)
                cout << "错误：" << err_msg << endl;
            else
                cout << "查找结果：" << endl << result;
        }
        else if (cmd_name == "help")
        {
            if (parts.size() == 1)
            {
                cout << left << setw(30) << "ls [-a] [-l] [-h] [路径]" << "列出目录内容" << endl;
                cout << left << setw(30) << "cat [-n] [-b] [-s] <文件名>" << "查看文件内容" << endl;
                cout << left << setw(30) << "cp [-r/-R] <源路径> <目标路径>" << "复制文件/目录" << endl;
                cout << left << setw(30) << "rm [-r/-R] <路径>" << "删除文件/目录" << endl;
                cout << left << setw(30) << "mv <源路径> <目标路径>" << "移动/重命名文件/目录" << endl;
                cout << left << setw(30) << "mkdir [-p] <目录路径>" << "创建目录" << endl;
                cout << left << setw(30) << "rmdir [-p] <目录路径>" << "删除目录" << endl;
                cout << left << setw(30) << "cd <目录路径>" << "切换工作目录" << endl;
                cout << left << setw(30) << "find <查找目录> <匹配模式（文件名）>" << "查找文件" << endl;
                cout << left << setw(30) << "help [命令名]" << "查看帮助信息" << endl;
                cout << left << setw(30) << "exit" << "退出" << endl;
            }
            else if (parts.size() == 2)
            {
                //string cmd_name0 = parts[1];

                if (parts[1] == "ls")
                {
                    cout << "语法：ls [-a] [-l] [-h] [路径]" << endl;
                    cout << "功能：列出指定目录的文件和子目录（默认当前目录）" << endl;
                    cout << "选项：" << endl;
                    cout << "  -a  显示所有文件（包括隐藏文件，以.开头的文件）" << endl;
                    cout << "  -l  长格式显示（包含名称、类型、大小、修改时间）" << endl;
                    cout << "  -h  以人类可读格式显示文件大小（如KB、MB，需配合-l使用）" << endl;
                }
                else if (parts[1] == "cat")
                {
                    cout << "语法：cat [-n] [-b] [-s] <文件名>" << endl;
                    cout << "功能：读取并显示文件内容" << endl;
                    cout << "选项：" << endl;
                    cout << "  -n  显示所有行的行号（包括空行）" << endl;
                    cout << "  -b  仅显示非空行的行号" << endl;
                    cout << "  -s  压缩连续的空行为一行" << endl;
                }
                else if (parts[1] == "cp")
                {
                    cout << "语法：cp [-r/-R] <源路径> <目标路径>" << endl;
                    cout << "功能：复制文件或目录（复制目录需加-r选项）" << endl;
                    cout << "选项：" << endl;
                    cout << "  -r/R  递归复制，用于复制目录（含子目录和文件）" << endl;
                }
                else if (parts[1] == "rm")
                {
                    cout << "语法：rm [-r/-R] <路径>" << endl;
                    cout << "功能：删除文件或目录（删除目录需加-r选项）" << endl;
                    cout << "选项：" << endl;
                    cout << "  -r/-R  递归删除，用于删除目录（含子目录和文件）" << endl;
                }
                else if (parts[1] == "mv")
                {
                    cout << "语法：mv <源路径> <目标路径>" << endl;
                    cout << "功能：移动文件/目录，或重命名文件/目录" << endl;
                }
                else if (parts[1] == "mkdir")
                {
                    cout << "语法：mkdir [-p] <目录路径>" << endl;
                    cout << "功能：创建新目录" << endl;
                    cout << "选项：" << endl;
                    cout << "  -p    递归创建父目录（如mkdir -p a/b/c，自动创建a和a/b）" << endl;
                }
                else if (parts[1] == "rmdir")
                {
                    cout << "语法：rmdir [-p] <目录路径>" << endl;
                    cout << "功能：删除空目录（删除非空目录需配合-p选项）" << endl;
                    cout << "选项：" << endl;
                    cout << "  -p    递归删除父目录（仅当父目录为空时）" << endl;
                }
                else if (parts[1] == "cd")
                {
                    cout << "语法：cd <目录路径>" << endl;
                    cout << "功能：切换当前工作目录" << endl;
                }
                else if (parts[1] == "find")
                {
                    cout << "语法：find <查找目录> <匹配模式（文件名）>" << endl;
                    cout << "功能：在指定目录下查找匹配名称的文件" << endl;
                }
                else if (parts[1] == "help")
                {
                    cout << "语法：help [命令名]" << endl;
                    cout << "功能：查看命令帮助信息" << endl;
                }
                else if (parts[1] == "exit")
                {
                    cout << "语法：exit" << endl;
                    cout << "功能：退出Windows兼容openEuler命令行接口" << endl;
                }
                else
                    cout << "错误：未知命令名'" << parts[1] << "'，请输入支持的命令名！" << endl;
            }
            else
                cout << "错误：参数错误" << endl;
        }
        else
            cout << "错误：未知命令'" << cmd_name << "'，请输入支持的命令！" << endl;
        cout << endl;
    }
    return 0;
}