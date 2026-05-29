#include "openEuler_cmd_core.h"
#include <stdlib.h>
#include <stdio.h>
void convert_path(char* path)// 路径转换
{
    if (path == NULL)
        return;
    for (int i = 0; path[i] != '\0'; i++)
        if (path[i] == '/')
            path[i] = '\\';
}
static void human_readable_size(DWORDLONG size, char* output) // 辅助函数：人类可读大小转换（B->KB/MB/GB）
{
    const char* units[] = { "B", "KB", "MB", "GB", "TB" };
    int unit_idx = 0;
    double size_d = (double)size;
    while (size_d >= 1024 && unit_idx < 4)
    {
        size_d /= 1024;
        unit_idx++;
    }
    sprintf_s(output, sizeof(output), "%.1f %s", size_d, units[unit_idx]);
}
int ls(const char* path, DirEntry** entries, int* count, char* err_msg, BOOL show_all, BOOL long_format, BOOL human_readable)
{
    char search_path[MAX_PATH];
    if (!path || strlen(path) == 0)// 处理默认路径（当前目录）
    {
        if (!GetCurrentDirectoryA(MAX_PATH, search_path))
        {
            strcpy_s(err_msg, 256, "获取当前目录失败");
            return CMD_ERROR_OTHER;
        }
    }
    else
    {
        strcpy_s(search_path, MAX_PATH, path);
        convert_path(search_path);
        DWORD attr = GetFileAttributesA(search_path);// 判断路径是否为目录
        if (attr == INVALID_FILE_ATTRIBUTES)
        {
            strcpy_s(err_msg, 256, "路径不存在");
            return CMD_ERROR_PATH;
        }
        if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
        {
            strcpy_s(err_msg, 256, "路径不是目录");
            return CMD_ERROR_PARAM;
        }
    }

    strcat_s(search_path, MAX_PATH, "\\*.*");//\*.*搜索所有文件和子目录
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        strcpy_s(err_msg, 256, "遍历目录失败");
        return CMD_ERROR_OTHER;
    }

    int entry_count = 0;// 统计目录项数量
    do
    {
        if (show_all)
        {
            if (strcmp(find_data.cFileName, ".") && strcmp(find_data.cFileName, ".."))// 跳过.和..
                entry_count++;
        }
        else
        {
            if (find_data.cFileName[0] != '.')
                entry_count++;
        }
    } while (FindNextFileA(hFind, &find_data));
    FindClose(hFind);

    *entries = (DirEntry*)malloc(entry_count * sizeof(DirEntry));// 分配目录项内存
    if (!*entries)
    {
        strcpy_s(err_msg, 256, "内存分配失败");
        return CMD_ERROR_OTHER;
    }

    hFind = FindFirstFileA(search_path, &find_data);// 填充目录项信息
    if (hFind == INVALID_HANDLE_VALUE)
    {
        free(*entries);
        *entries = NULL;
        strcpy_s(err_msg, 256, "遍历目录失败");
        return CMD_ERROR_OTHER;
    }

    int idx = 0;
    do
    {
        if (show_all)
        {
            if (strcmp(find_data.cFileName, ".") && strcmp(find_data.cFileName, ".."))
            {
                DirEntry* entry = &(*entries)[idx];
                strcpy_s(entry->filename, MAX_PATH, find_data.cFileName);
                if (long_format)
                {
                    entry->is_directory = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
                    entry->file_size = entry->is_directory ? 0 : (find_data.nFileSizeLow + ((DWORDLONG)find_data.nFileSizeHigh << 32));
                    if (human_readable)
                        human_readable_size(entry->file_size, entry->hfile_size);
                    entry->modify_time = find_data.ftLastWriteTime;
                }
                idx++;
            }
        }
        else
        {
            if (find_data.cFileName[0] != '.')
            {
                DirEntry* entry = &(*entries)[idx];
                strcpy_s(entry->filename, MAX_PATH, find_data.cFileName);
                if (long_format)
                {
                    entry->is_directory = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
                    entry->file_size = entry->is_directory ? 0 : (find_data.nFileSizeLow + ((DWORDLONG)find_data.nFileSizeHigh << 32));
                    if (human_readable)
                        human_readable_size(entry->file_size, entry->hfile_size);
                    entry->modify_time = find_data.ftLastWriteTime;
                }
                idx++;
            }
        }
    } while (FindNextFileA(hFind, &find_data) && idx < entry_count);

    FindClose(hFind);
    *count = entry_count;
    return CMD_SUCCESS;
}
static void cat_process(char* buffer, int buffer_size, BOOL show_number, BOOL show_number_nonblank, BOOL squeeze_blank)// 辅助函数：处理行号和空行压缩
{
    char temp_buffer[8192]; // 扩大临时缓冲区
    memset(temp_buffer, 0, sizeof(temp_buffer));
    int temp_idx = 0;
    int line_num = 1;
    BOOL prev_blank = FALSE;
    char* context = NULL;

    char* line = strtok_s(buffer, "\n", &context);
    while (line != NULL && temp_idx < sizeof(temp_buffer) - 1)
    {
        BOOL is_blank = TRUE;
        for (size_t i = 0; i < strlen(line); i++)
        {
            if (line[i] != ' ' && line[i] != '\t' && line[i] != '\r')
                is_blank = FALSE;
            break;
        }
        if (squeeze_blank)// 处理 -s：压缩连续空行
        {
            if (is_blank && prev_blank)
            {
                line = strtok_s(NULL, "\n", &context);
                continue;
            }
            prev_blank = is_blank;
        }
        int write_bytes = 0;
        if (show_number_nonblank)// 处理行号：-b 优先级高于 -n
            if (!is_blank)
                write_bytes = sprintf_s(temp_buffer + temp_idx, sizeof(temp_buffer) - temp_idx - 1, "%6d\t%s\n", line_num++, line);
            else
                write_bytes = sprintf_s(temp_buffer + temp_idx, sizeof(temp_buffer) - temp_idx - 1, "%6c\n", ' ');
        else if (show_number)
            write_bytes = sprintf_s(temp_buffer + temp_idx, sizeof(temp_buffer) - temp_idx - 1, "%6d\t%s\n", line_num++, line);
        else
            write_bytes = sprintf_s(temp_buffer + temp_idx, sizeof(temp_buffer) - temp_idx - 1, "%s\n", line);
        if (write_bytes < 0)
            break;
        temp_idx += write_bytes;
        line = strtok_s(NULL, "\n", &context);
    }
    temp_buffer[temp_idx] = '\0';
    strncpy_s(buffer, buffer_size, temp_buffer, _TRUNCATE);// 复制回原缓冲区（防止溢出）
}
int cat(const char* filename, char* buffer, int buffer_size, char* err_msg, BOOL show_number, BOOL show_number_nonblank, BOOL squeeze_blank)
{
    char file_path[MAX_PATH];
    strcpy_s(file_path, MAX_PATH, filename);
    convert_path(file_path);

    DWORD attr = GetFileAttributesA(file_path);// 验证文件存在且不是目录
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        strcpy_s(err_msg, 256, "文件不存在");
        return CMD_ERROR_FILE;
    }
    if (attr & FILE_ATTRIBUTE_DIRECTORY)
    {
        strcpy_s(err_msg, 256, "无法读取目录");
        return CMD_ERROR_PARAM;
    }

    HANDLE hFile = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);// 打开并读取文件
    if (hFile == INVALID_HANDLE_VALUE)
    {
        strcpy_s(err_msg, 256, "打开文件失败（权限不足或文件被占用）");
        return CMD_ERROR_PERM;
    }

    DWORD bytes_read;
    BOOL success = ReadFile(hFile, buffer, buffer_size - 1, &bytes_read, NULL);
    buffer[bytes_read] = '\0';
    CloseHandle(hFile);

    if (!success)
    {
        strcpy_s(err_msg, 256, "读取文件失败");
        return CMD_ERROR_OTHER;
    }
    if (show_number || show_number_nonblank || squeeze_blank)
        cat_process(buffer, buffer_size, show_number, show_number_nonblank, squeeze_blank);
    return CMD_SUCCESS;
}
static int cp_dir_recursive(const char* src_dir, const char* dest_dir, char* err_msg)
{
    if (mkdir(dest_dir, err_msg, FALSE) != CMD_SUCCESS)// 创建目标目录
    {
        DWORD attr = GetFileAttributesA(dest_dir);
        if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY))
            return CMD_ERROR_OTHER;
    }

    // 遍历源目录
    char search_path[MAX_PATH];
    strcpy_s(search_path, MAX_PATH, src_dir);
    strcat_s(search_path, MAX_PATH, "\\*.*");
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        strcpy_s(err_msg, 256, "遍历目录失败");
        return CMD_ERROR_OTHER;
    }

    do
    {
        if (strcmp(find_data.cFileName, ".") && strcmp(find_data.cFileName, ".."))
        {
            char src_path[MAX_PATH] = { 0 }, dest_path[MAX_PATH] = { 0 };
            sprintf_s(src_path, MAX_PATH, "%s\\%s", src_dir, find_data.cFileName);
            sprintf_s(dest_path, MAX_PATH, "%s\\%s", dest_dir, find_data.cFileName);

            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // 递归复制子目录
                if (cp_dir_recursive(src_path, dest_path, err_msg) != CMD_SUCCESS)
                {
                    FindClose(hFind);
                    return CMD_ERROR_OTHER;
                }
            }
            else
            {
                // 复制文件
                if (!CopyFileA(src_path, dest_path, FALSE))
                {
                    strcpy_s(err_msg, 256, "复制文件失败（权限不足或被占用）");
                    FindClose(hFind);
                    return CMD_ERROR_OTHER;
                }
            }
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
    return CMD_SUCCESS;
}
int cp(const char* src_path, const char* dest_path, char* err_msg, BOOL recursive)
{
    char src[MAX_PATH], dest[MAX_PATH];
    strcpy_s(src, MAX_PATH, src_path);
    strcpy_s(dest, MAX_PATH, dest_path);
    convert_path(src);
    convert_path(dest);

    DWORD src_attr = GetFileAttributesA(src);
    if (src_attr == INVALID_FILE_ATTRIBUTES)
    {
        strcpy_s(err_msg, 256, "源文件/目录不存在");
        return CMD_ERROR_FILE;
    }
    if (src_attr & FILE_ATTRIBUTE_DIRECTORY)
    {// 源是目录
        if (!recursive)
        {
            strcpy_s(err_msg, 256, "源是目录，请使用 -r 选项递归复制");
            return CMD_ERROR_PARAM;
        }

        // 处理目标路径：如果目标存在且是目录，复制到目标下的子目录
        DWORD dest_attr = GetFileAttributesA(dest);
        if (dest_attr != INVALID_FILE_ATTRIBUTES)
        {
            if (!(dest_attr & FILE_ATTRIBUTE_DIRECTORY))
            {
                strcpy_s(err_msg, 256, "目标存在且不是目录");
                return CMD_ERROR_PARAM;
            }
            char src_dir_name[MAX_PATH];
            errno_t split_ret = _splitpath_s(src, NULL, 0, NULL, 0, src_dir_name, MAX_PATH, NULL, 0);
            if (split_ret != 0)
            {
                strcpy_s(err_msg, 256, "提取文件名失败");
                return CMD_ERROR_OTHER;
            }
            strcat_s(dest, MAX_PATH, "\\");
            strcat_s(dest, MAX_PATH, src_dir_name);
        }

        return cp_dir_recursive(src, dest, err_msg);
    }
    else
    {// 源是文件
        DWORD dest_attr = GetFileAttributesA(dest);
        if (dest_attr != INVALID_FILE_ATTRIBUTES && (dest_attr & FILE_ATTRIBUTE_DIRECTORY))// 判断目标路径是否为目录
        { // 从源路径中提取完整文件名
            char src_fname[MAX_PATH];  // 文件名
            char src_ext[MAX_PATH];    // 后缀
            errno_t split_ret = _splitpath_s(src, NULL, 0, NULL, 0, src_fname, MAX_PATH, src_ext, MAX_PATH);// 拆分路径：驱动器(忽略)、目录(忽略)、文件名、后缀
            if (split_ret != 0)
            {
                strcpy_s(err_msg, 256, "提取文件名失败");
                return CMD_ERROR_OTHER;
            }

            char full_filename[MAX_PATH];// 拼接文件名+后缀
            strcpy_s(full_filename, MAX_PATH, src_fname);
            strcat_s(full_filename, MAX_PATH, src_ext);
            strcat_s(dest, MAX_PATH, "\\");
            strcat_s(dest, MAX_PATH, full_filename);
        }

        if (!CopyFileA(src, dest, FALSE))// 复制文件
        {
            strcpy_s(err_msg, 256, "复制失败（目标路径无效或权限不足）");
            return CMD_ERROR_OTHER;
        }
        return CMD_SUCCESS;
    }
}
static int rm_dir_recursive(const char* dir_path, char* err_msg)// 辅助函数：递归删除目录内容
{
    char search_path[MAX_PATH];
    strcpy_s(search_path, MAX_PATH, dir_path);
    strcat_s(search_path, MAX_PATH, "\\*.*");
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        strcpy_s(err_msg, 256, "遍历目录失败");
        return CMD_ERROR_OTHER;
    }

    do
    {
        if (strcmp(find_data.cFileName, ".") && strcmp(find_data.cFileName, ".."))
        {
            char full_path[MAX_PATH] = { 0 };
            sprintf_s(full_path, MAX_PATH, "%s\\%s", dir_path, find_data.cFileName);

            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // 递归删除子目录
                if (rm_dir_recursive(full_path, err_msg) != CMD_SUCCESS)
                {
                    FindClose(hFind);
                    return CMD_ERROR_OTHER;
                }
                if (!RemoveDirectoryA(full_path))
                {
                    strcpy_s(err_msg, 256, "删除子目录失败（权限不足或被占用）");
                    FindClose(hFind);
                    return CMD_ERROR_OTHER;
                }
            }
            else
            {
                // 删除文件
                if (!DeleteFileA(full_path))
                {
                    strcpy_s(err_msg, 256, "删除文件失败（权限不足或被占用）");
                    FindClose(hFind);
                    return CMD_ERROR_OTHER;
                }
            }
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
    return CMD_SUCCESS;
}
int rm(const char* path, char* err_msg, BOOL recursive)
{
    char file_path[MAX_PATH];
    strcpy_s(file_path, MAX_PATH, path);
    convert_path(file_path);

    DWORD attr = GetFileAttributesA(file_path);// 验证文件存在且不是目录
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        strcpy_s(err_msg, 256, "文件/目录不存在");
        return CMD_ERROR_FILE;
    }
    if (attr & FILE_ATTRIBUTE_DIRECTORY)// 目标是目录：必须带 -r 选项
    {
        if (!recursive)
        {
            strcpy_s(err_msg, 256, "目标是目录，请使用 -r 选项递归删除");
            return CMD_ERROR_PARAM;
        }

        // 递归删除目录内容
        if (rm_dir_recursive(file_path, err_msg) != CMD_SUCCESS)
            return CMD_ERROR_OTHER;

        // 删除空目录
        if (!RemoveDirectoryA(file_path))
        {
            strcpy_s(err_msg, 256, "删除目录失败（权限不足或被占用）");
            return CMD_ERROR_OTHER;
        }
        return CMD_SUCCESS;
    }
    else// 目标是文件
    {
        if (!DeleteFileA(file_path))// 删除文件
        {
            strcpy_s(err_msg, 256, "删除失败（文件被占用或权限不足）");
            return CMD_ERROR_OTHER;
        }
        return CMD_SUCCESS;
    }
}
int mv(const char* src_path, const char* dest_path, char* err_msg)
{
    char src[MAX_PATH], dest[MAX_PATH];
    strcpy_s(src, MAX_PATH, src_path);
    strcpy_s(dest, MAX_PATH, dest_path);
    convert_path(src);
    convert_path(dest);

    DWORD src_attr = GetFileAttributesA(src);// 验证源路径存在
    if (src_attr == INVALID_FILE_ATTRIBUTES)
    {
        strcpy_s(err_msg, 256, "源路径不存在");
        return CMD_ERROR_PATH;
    }
    BOOL src_is_dir = (src_attr & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;

    DWORD dest_attr = GetFileAttributesA(dest);// 处理目标路径
    if (dest_attr != INVALID_FILE_ATTRIBUTES)
    {
        BOOL dest_is_dir = (dest_attr & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;// 目标路径已存在：判断是否为目录
        if (dest_is_dir)
        { // 提取源路径的完整文件名
            char src_fname[MAX_PATH];  // 文件名
            char src_ext[MAX_PATH];    // 后缀
            errno_t split_ret = _splitpath_s(src, NULL, 0, NULL, 0, src_fname, MAX_PATH, src_ext, MAX_PATH);
            if (split_ret != 0)
            {
                strcpy_s(err_msg, 256, "提取源文件名失败");
                return CMD_ERROR_OTHER;
            }
            char full_filename[MAX_PATH];// 拼接文件名和后缀
            strcpy_s(full_filename, MAX_PATH, src_fname);
            strcat_s(full_filename, MAX_PATH, src_ext);
            strcat_s(dest, MAX_PATH, "\\");
            strcat_s(dest, MAX_PATH, full_filename);

            if (GetFileAttributesA(dest) != INVALID_FILE_ATTRIBUTES)// 同名文件检查
            {
                strcpy_s(err_msg, 256, "目标目录下已存在同名文件，无法覆盖");
                return CMD_ERROR_OTHER;
            }
        }
    }// 目标路径不存在，直接重命名

    if (!MoveFileA(src, dest))// 执行移动/重命名操作
    {
        DWORD err = GetLastError();
        switch (err)
        {
        case ERROR_ACCESS_DENIED:
            strcpy_s(err_msg, 256, "移动失败（权限不足）");
            break;
        case ERROR_FILE_NOT_FOUND:
            strcpy_s(err_msg, 256, "移动失败（源文件不存在）");
            break;
        case ERROR_SHARING_VIOLATION:
            strcpy_s(err_msg, 256, "移动失败（文件被其他程序占用）");
            break;
        default:
            strcpy_s(err_msg, 256, "移动失败（目标路径无效或系统错误）");
            break;
        }
        return CMD_ERROR_OTHER;
    }
    return CMD_SUCCESS;
}
int mkdir(const char* path, char* err_msg, BOOL create_parent)
{
    char dir_path[MAX_PATH];
    strcpy_s(dir_path, MAX_PATH, path);
    convert_path(dir_path);

    if (create_parent)// 递归模式：拆分路径并逐级创建
    {
        char temp_path[MAX_PATH];
        strcpy_s(temp_path, MAX_PATH, dir_path);
        int len = (int)strlen(temp_path);
        if (len > 0 && temp_path[len - 1] == '\\') temp_path[len - 1] = '\0';

        for (int i = 0; temp_path[i] != '\0'; i++)
        {
            if (temp_path[i] == '\\' && i > 0)
            {
                char curr_dir[MAX_PATH];
                strncpy_s(curr_dir, MAX_PATH, temp_path, i);
                curr_dir[i] = '\0';

                DWORD attr = GetFileAttributesA(curr_dir);
                if (attr == INVALID_FILE_ATTRIBUTES)
                {
                    if (!CreateDirectoryA(curr_dir, NULL))
                    {
                        strcpy_s(err_msg, 256, "创建上级目录失败（权限不足）");
                        return CMD_ERROR_OTHER;
                    }
                }
                else if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
                {
                    strcpy_s(err_msg, 256, "上级路径存在且不是目录");
                    return CMD_ERROR_OTHER;
                }
            }
        }
    }

    DWORD attr = GetFileAttributesA(dir_path);// 判断目标路径是否已存在
    if (attr != INVALID_FILE_ATTRIBUTES)
    {
        if (attr & FILE_ATTRIBUTE_DIRECTORY)// 路径已存在：判断是否为目录
            strcpy_s(err_msg, 256, "目录已存在");
        else
            strcpy_s(err_msg, 256, "路径已存在且不是目录");
        return CMD_ERROR_OTHER;
    }

    if (!CreateDirectoryA(dir_path, NULL))// 创建目录
    {
        DWORD err = GetLastError();
        switch (err)
        {
        case ERROR_ACCESS_DENIED:
            strcpy_s(err_msg, 256, "创建失败（权限不足，无法写入目标路径）");
            break;
        case ERROR_PATH_NOT_FOUND:
            strcpy_s(err_msg, 256, "创建失败（上级目录不存在）");
            break;
        default:
            strcpy_s(err_msg, 256, "创建失败（路径无效或系统错误）");
            break;
        }
        return CMD_ERROR_OTHER;
    }
    return CMD_SUCCESS;
}
int rmdir(const char* path, char* err_msg, BOOL remove_parent, BOOL* first_rm)
{
    char dir_path[MAX_PATH];
    strcpy_s(dir_path, MAX_PATH, path);
    convert_path(dir_path);

    DWORD attr = GetFileAttributesA(dir_path);// 验证目录存在且为空
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        strcpy_s(err_msg, 256, "目录不存在");
        return CMD_ERROR_PATH;
    }
    if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
    {
        strcpy_s(err_msg, 256, "路径不是目录");
        return CMD_ERROR_PARAM;
    }

    char search_path[MAX_PATH];// 检查目录是否为空
    strcpy_s(search_path, MAX_PATH, dir_path);
    strcat_s(search_path, MAX_PATH, "\\*.*");
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        strcpy_s(err_msg, 256, "检查目录失败");
        return CMD_ERROR_OTHER;
    }

    BOOL is_empty = TRUE;
    do
    {
        if (strcmp(find_data.cFileName, ".") && strcmp(find_data.cFileName, ".."))
        {
            is_empty = FALSE;
            break;
        }
    } while (FindNextFileA(hFind, &find_data));
    FindClose(hFind);

    if (!is_empty)
    {
        if (*first_rm)
            strcpy_s(err_msg, 256, "目录不为空，无法删除");
        return CMD_ERROR_OTHER;
    }

    if (!RemoveDirectoryA(dir_path))// 删除目录
    {
        strcpy_s(err_msg, 256, "删除失败（权限不足或目录被占用）");
        return CMD_ERROR_OTHER;
    }
    *first_rm = FALSE;

    if (remove_parent) // 递归删除上级空目录
    {
        char parent_dir[MAX_PATH];
        strcpy_s(parent_dir, MAX_PATH, dir_path);
        char* last_slash = strrchr(parent_dir, '\\');
        if (last_slash == NULL || last_slash == parent_dir) // 根目录终止
            return CMD_SUCCESS;
        *last_slash = '\0';
        return rmdir(parent_dir, err_msg, TRUE, first_rm);
    }
    return CMD_SUCCESS;
}
int cd(const char* path, char* current_dir, int dir_len, char* err_msg)
{
    char dir_path[MAX_PATH];
    strcpy_s(dir_path, MAX_PATH, path);
    convert_path(dir_path);

    if (!SetCurrentDirectoryA(dir_path))// 切换目录
    {
        strcpy_s(err_msg, 256, "切换失败（路径不存在或权限不足）");
        return CMD_ERROR_PATH;
    }

    if (!GetCurrentDirectoryA(dir_len, current_dir))// 获取当前目录并返回
    {
        strcpy_s(err_msg, 256, "获取当前目录失败");
        return CMD_ERROR_OTHER;
    }
    return CMD_SUCCESS;
}
static void find_recursive(const char* root, const char* filename, char* result, int* result_len, int result_size)// find递归辅助函数
{
    if (!root || !filename || !result || !result_len || result_size <= 0)
        return;
    char search_path[MAX_PATH];
    strcpy_s(search_path, MAX_PATH, root);
    strcat_s(search_path, MAX_PATH, "\\*.*");
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE)
        return;
    do {
        if (strcmp(find_data.cFileName, ".") && strcmp(find_data.cFileName, ".."))
        {
            char full_path[MAX_PATH];
            strcpy_s(full_path, MAX_PATH, root);
            strcat_s(full_path, MAX_PATH, "\\");
            strcat_s(full_path, MAX_PATH, find_data.cFileName);

            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                find_recursive(full_path, filename, result, result_len, result_size);// 递归遍历子目录
            else if (!strcmp(find_data.cFileName, filename))
            {
                int len = (int)strlen(full_path);// 匹配文件名，写入结果
                if (*result_len + len + 2 <= result_size)
                {
                    strcpy_s(result + *result_len, result_size - *result_len, full_path);
                    *result_len += len;
                    result[(*result_len)++] = '\n';
                    result[*result_len] = '\0';
                }
            }
        }
    } while (FindNextFileA(hFind, &find_data));
    FindClose(hFind);
}
int find(const char* root_path, const char* filename, char* result, int result_size, char* err_msg)
{
    char root[MAX_PATH];
    strcpy_s(root, MAX_PATH, root_path);
    convert_path(root);

    DWORD attr = GetFileAttributesA(root);// 验证根目录存在
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        strcpy_s(err_msg, 256, "根目录不存在");
        return CMD_ERROR_PATH;
    }
    if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
    {
        strcpy_s(err_msg, 256, "根路径不是目录");
        return CMD_ERROR_PARAM;
    }

    result[0] = '\0';// 初始化结果缓冲区
    int result_len = 0;
    find_recursive(root, filename, result, &result_len, result_size);

    if (result_len == 0)
        strcpy_s(result, result_size, "未找到目标文件\n");
    return CMD_SUCCESS;
}