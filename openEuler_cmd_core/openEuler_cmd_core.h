#ifndef OPENEULER_CMD_CORE_H
#define OPENEULER_CMD_CORE_H
#include <windows.h>
#include <string.h>
#ifdef __cplusplus
extern "C"
{
#endif
#define CMD_SUCCESS 0          // 成功
#define CMD_ERROR_PATH 1       // 路径错误
#define CMD_ERROR_FILE 2       // 文件错误
#define CMD_ERROR_PERM 3       // 权限错误
#define CMD_ERROR_PARAM 4      // 参数错误
#define CMD_ERROR_OTHER 5      // 其他错误
    typedef struct// 目录项，用于存储文件/目录元信息
    {
        char filename[MAX_PATH];  // 文件名（含后缀）
        DWORD file_size;          // 文件大小（字节），目录为0
        char hfile_size[MAX_PATH];
        BOOL is_directory;        // 是否为目录（TRUE/FALSE）
        FILETIME modify_time;     // 最后修改时间
    } DirEntry;
    // 路径转换：将openEuler的'/'转为Windows的'\'
    void convert_path(char* path);
    // ls命令：列出目录内容 path：目录路径（NULL=当前目录），entries：输出目录项数组，count：输出条目数，err_msg：错误信息（256字节）
    int ls(const char* path, DirEntry** entries, int* count, char* err_msg, BOOL show_all, BOOL long_format, BOOL human_readable);
    // cat命令：读取文件内容 filename：目标文件，buffer：输出缓冲区，buffer_size：缓冲区大小，err_msg：错误信息
    int cat(const char* filename, char* buffer, int buffer_size, char* err_msg, BOOL show_number, BOOL show_number_nonblank, BOOL squeeze_blank);
    // cp命令：复制文件 src_path：源文件，dest_path：目标文件，err_msg：错误信息
    int cp(const char* src_path, const char* dest_path, char* err_msg, BOOL recursive);
    // rm命令：删除文件 path：目标文件路径，err_msg：错误信息
    int rm(const char* path, char* err_msg, BOOL recursive);
    // mv命令：移动/重命名文件/目录 src_path：源路径，dest_path：目标路径，err_msg：错误信息
    int mv(const char* src_path, const char* dest_path, char* err_msg);
    // mkdir命令：创建目录（避免与系统宏冲突）
    int mkdir(const char* path, char* err_msg, BOOL create_parent);
    // rmdir命令：删除空目录
    int rmdir(const char* path, char* err_msg, BOOL remove_parent, BOOL* first_rm);
    // cd命令：切换当前目录 path：目标目录，current_dir：输出当前目录（MAX_PATH字节），err_msg：错误信息
    int cd(const char* path, char* current_dir, int dir_len, char* err_msg);
    // find命令：递归查找文件（精确匹配） root_path：查找根目录，filename：目标文件名，result：输出路径（8192字节），err_msg：错误信息
    int find(const char* root_path, const char* filename, char* result, int result_size, char* err_msg);
    static void human_readable_size(DWORDLONG size, char* output);
    static void cat_process(char* buffer, int buffer_size, BOOL show_number, BOOL show_number_nonblank, BOOL squeeze_blank);
    static int cp_dir_recursive(const char* src_dir, const char* dest_dir, char* err_msg);
    static int rm_dir_recursive(const char* dir_path, char* err_msg);
    static void find_recursive(const char* root, const char* filename, char* result, int* result_len, int result_size);
#ifdef __cplusplus
}
#endif
#endif // OPENEULER_CMD_CORE_H