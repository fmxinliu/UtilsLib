#include "StdAfx.h"
#include "path.h"
#include "StringUtils.h"
#include <stdio.h>
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#include <direct.h>
#define F_OK  0 // 文件存在
#define W_OK  2 // 写权限
#define R_OK  4 // 读权限
#define RW_OK 6 // 读写权限
#pragma warning(disable: 4996)

#ifdef UNICODE
#define finddata_t _wfinddata_t
#define findfirst  _wfindfirst
#define findnext   _wfindnext
#else
#define finddata_t _finddata_t
#define findfirst  _findfirst
#define findnext   _findnext
#endif

#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef UNICODE
#define _T(x)  L ## x
typedef wchar_t TCHAR;
#define _tcscat       wcscat
#define _tcscmp       wcscmp
#define _tcscpy       wcscpy
#define _tcslen       wcslen
#define _sntprintf    snwprintf
#define _tsplitpath   _wsplitpath

#else
#define _T(x)  x
typedef char TCHAR;
#define _tcscat       strcat
#define _tcscmp       strcmp
#define _tcscpy       strcpy
#define _tcslen       strlen
#define _sntprintf    snprintf
#define _tsplitpath   _splitpath

#endif
#endif

using namespace std;
namespace UtilTools
{
    int _access(const String &path, int accessMode)
    {
#ifdef UNICODE
        //WS2S_PTR(path, ptr);
        //return access(ptr, accessMode);
        string s;
        return access(StringUtils::wstringToString(path, s).c_str(), accessMode);
#else
        return access(path.c_str(), accessMode);
#endif
    }

    int _chdir(const String &path)
    {
#ifdef UNICODE
        WS2S_PTR(path, ptr);
        return chdir(ptr);
#else
        return chdir(path.c_str());
#endif
    }

    int _mkdir(const String &path)
    {
#ifdef UNICODE
        WS2S_PTR(path, ptr);
        return mkdir(ptr);
#else
        return mkdir(path.c_str());
#endif
    }

    int _rmdir(const String &path)
    {
#ifdef UNICODE
        WS2S_PTR(path, ptr);
        return rmdir(ptr);
#else
        return rmdir(path.c_str());
#endif
    }

    int _rename(const String &oldname, const String &newname)
    {
#ifdef UNICODE
        WS2S_PTR(oldname, ptr1);
        WS2S_PTR(newname, ptr2);
        return std::rename(ptr1, ptr2);
#else
        return std::rename(oldname.c_str(), newname.c_str());
#endif
    }

    int _remove(const String &path)
    {
#ifdef UNICODE
        WS2S_PTR(path, ptr);
        return remove(ptr);
#else
        return remove(path.c_str());
#endif
    }

    String _getcwd()
    {
        char szCurWorkDir[MAX_PATH] = {};
        //GetCurrentDirectory(MAX_PATH, szCurWorkDir);
#ifdef UNICODE
        return StringUtils::stringToWString(getcwd(szCurWorkDir, MAX_PATH));
#else
        return getcwd(szCurWorkDir, MAX_PATH);
#endif
    }

    int _system(const String &command)
    {
#ifdef UNICODE
        WS2S_PTR(command, ptr);
        return system(ptr);
#else
        return system(command.c_str());
#endif
    }

    String parseTime(time_t rawtime)
    {
        TCHAR buffer[80];
        struct tm *info = localtime(&rawtime);

#ifdef UNICODE
        wcsftime(buffer, 80, L"%Y-%m-%d %H:%M:%S", info);
#else
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", info);
#endif
        return String(buffer);
    }
}

namespace UtilTools
{
#if defined(WIN32)
    bool listFolders(const String &pathname, bool listhiddenfolders, vector<String> &result)
    {
        long hFile = 0;
        struct finddata_t fileInfo;
        String pathName = StringUtils::endsWith(pathname, _T("\\")) ? pathName : pathname + _T("\\");

        if ((hFile = findfirst((pathName + _T("*")).c_str(), &fileInfo)) == -1) {
            return false;
        }

        result.push_back(pathname);

        // List all subdirectory
        do {
            if (!_tcscmp(fileInfo.name, _T(".")) || !_tcscmp(fileInfo.name, _T(".."))) {
                continue;
            } else if (fileInfo.attrib & _A_SUBDIR) {
                String subdir = pathName + fileInfo.name;
                if ((GetFileAttributes(subdir.c_str()) & FILE_ATTRIBUTE_HIDDEN) == 0 || listhiddenfolders) {
                    listFolders(subdir, listhiddenfolders, result);
                }
            }
        } while (findnext(hFile, &fileInfo) == 0);
        _findclose(hFile);
        return true;
    }

    bool listFiles(const String &pathname, bool listsubdir, bool listhiddenfiles, vector<String> &result)
    {
        long hFile = 0;
        struct finddata_t fileInfo;
        String pathName = StringUtils::endsWith(pathname, _T("\\")) ? pathName : pathname + _T("\\");

        if ((hFile = findfirst((pathName + _T("*")).c_str(), &fileInfo)) == -1) {
            return false;
        }
        // List all the files in the directory with some info about them.
        do {
            if (!_tcscmp(fileInfo.name, _T(".")) || !_tcscmp(fileInfo.name, _T(".."))) {
                continue;
            } else if (fileInfo.attrib & _A_SYSTEM) {
                continue; // 系统文件
            } else if (!listhiddenfiles && fileInfo.attrib & _A_HIDDEN) {
                continue;
            } else if (listsubdir &&  (fileInfo.attrib & _A_SUBDIR)) {
                listFiles(pathName + fileInfo.name, true, listhiddenfiles, result);
            } else {
                result.push_back(pathName + fileInfo.name);
            }
            //cout << fileInfo.name << (fileInfo.attrib&_A_SUBDIR? "[folder]":"[file]") << endl;
        } while (findnext(hFile, &fileInfo) == 0);
        _findclose(hFile);
        return true;
    }

    bool deleteDirectory(const String &pathname)
    {
        long hFile = 0;

        struct finddata_t fileInfo;
        String pathName = StringUtils::endsWith(pathname, _T("\\")) ? pathname : pathname + _T("\\");

        if ((hFile = findfirst((pathName + _T("*")).c_str(), &fileInfo)) == -1) {
            return false;
        }
        // Recursively delete all the file in the directory.
        do {
            if (!_tcscmp(fileInfo.name, _T(".")) || !_tcscmp(fileInfo.name, _T(".."))) {
                continue;
            } else if ((fileInfo.attrib & _A_SUBDIR)) {
                deleteDirectory(pathName + fileInfo.name);
            } else {
                _remove(pathName + fileInfo.name);
            }
        } while (findnext(hFile, &fileInfo) == 0);
        _findclose(hFile);
        return !_rmdir(pathname); // 删除空目录
    }

#else
    bool listFolders(const String &pathname, bool listhiddenfolders, vector<String> &result)
    {
        DIR *dirp;
        if((dirp = opendir(pathname.c_str()) == NULL) {
            return false;
        }

        string dir_full_path = endsWith(pathname, _T("/")) ? pathname : pathname + _T("/");
        struct dirent *dir;
        struct stat st;

        result.push_back(pathname);

        // Recursively all subdirectory.
        while ((dir = readdir(dirp)) != NULL) {
            if (!_tcscmp(dir->d_name, _T(".")) || !_tcscmp(dir->d_name, _T(".."))) {
                continue;
            }

            String sub_path = dir_full_path + dir->d_name;
            if (lstat(sub_path.c_str(), &st) == -1) {
                continue;
            } else if (S_ISDIR(st.st_mode)) {
                if (!StringUtils::startsWith(dir->d_name, _T(".") || listhiddenfolders) {
                    listFolders(sub_path, listhiddenfolders, result);
                }
            } else {
                result.push_back(sub_path);
            }
        }
        closedir(dirp);
        return true;
    }

    bool listFiles(const String &pathname, bool listsubdir, bool listhiddenfiles, vector<String> &result)
    {
        DIR *dirp;
        if((dirp = opendir(pathname.c_str()) == NULL) {
            return false;
        }

        string dir_full_path = endsWith(pathname, _T("/")) ? pathname : pathname + _T("/");
        struct dirent *dir;
        struct stat st;

        // Recursively find all the file in the directory.
        while ((dir = readdir(dirp)) != NULL) {
            if (!_tcscmp(dir->d_name, _T(".")) || !_tcscmp(dir->d_name, _T(".."))) {
                continue;
            }

            String sub_path = dir_full_path + dir->d_name;
            if (lstat(sub_path.c_str(), &st) == -1) {
                continue;
            } else if (!listhiddenfiles && StringUtils::startsWith(dir->d_name, _T("."))) {
                continue;
            } else if (listsubdir && S_ISDIR(st.st_mode)) {
                listFiles(sub_path, true, listhiddenfiles, result);
            } else {
                result.push_back(sub_path);
            }
        }
        closedir(dirp);
        return true;
    }

    bool deleteDirectory(const String &pathname)
    {
        DIR *dirp;
        if((dirp = opendir(pathname.c_str()) == NULL) {
            return false;
        }

        string dir_full_path = endsWith(pathname, _T("/")) ? pathname : pathname + _T("/");
        struct dirent *dir;
        struct stat st;

        // Recursively delete all the file in the directory.
        while ((dir = readdir(dirp)) != NULL) {
            if (!_tcscmp(dir->d_name, _T(".")) || !_tcscmp(dir->d_name, _T(".."))) {
                continue;
            }

            String sub_path = dir_full_path + dir->d_name;
            if (lstat(sub_path.c_str(), &st) == -1) {
                continue;
            } else if (S_ISDIR(st.st_mode)) {
                deleteDirectory(sub_path); // 如果是目录文件，递归删除
            }
            else if (S_ISREG(st.st_mode)) {
                unlink(sub_path.c_str()); // 如果是普通文件，则 unlink
            }
        }
        closedir(dirp);
        return !rmdir(pathname.c_str()); // 删除空目录
    }

#define SETNUL(pf) \
    do { \
        if (NULL != pf) { \
            pf[0] = '\0'; \
        } \
    } while(0)

    void _split_whole_name(const TCHAR *whole_name, TCHAR *fname, TCHAR *ext)
    {
        TCHAR *p_ext;

        p_ext = rindex(whole_name, '.');
        if (NULL != p_ext) {
            if (NULL != ext) {
                _tcscpy(ext, p_ext);
            }

            if (NULL != fname) {
                _sntprintf(fname, p_ext - whole_name + 1, _T("%s"), whole_name);
            }
        } else {
            SETNUL(ext);
            if (NULL != fname) {
                _tcscpy(fname, whole_name);
            }
        }
    }

    void _tsplitpath(const TCHAR *path, TCHAR *drive, TCHAR *dir, TCHAR *fname, TCHAR *ext)
    {
        TCHAR *p_whole_name;

        SETNUL(drive);
        if (NULL == path) {
            SETNUL(dir);
            SETNUL(fname);
            SETNUL(ext);
            return;
        }

        if ('/' == path[_tcslen(path)])  {
            if (NULL != dir) {
                strcpy(dir, path);
            }
            SETNUL(fname);
            SETNUL(ext);
            return;
        }

        p_whole_name = rindex(path, '/');
        if (NULL != p_whole_name) {
            p_whole_name++;
            _split_whole_name(p_whole_name, fname, ext);
            if (NULL != dir) {
                _sntprintf(dir, p_whole_name - path, _T("%s"), path);
            }
        } else {
            _split_whole_name(path, fname, ext);
            SETNUL(dir);
        }
    }
#endif
}

namespace UtilTools
{
    bool Path::isExist(const String &path)
    {
        return !_access(path, F_OK);
    }

    bool Path::isReadable(const String &path)
    {
        return !_access(path, R_OK);
    }

    bool Path::isWriteable(const String &path)
    {
        return !_access(path, W_OK);
    }

    bool Path::isExecutable(const String &path)
    {
#if defined(WIN32)
        return StringUtils::endsWith(StringUtils::toLower(StringUtils::trimRight(path)), _T(".exe"));
#else
        return !access(path.c_str(), X_OK);
#endif
    }

    bool Path::isReadwrite(const String &path)
    {
        return isReadable(path) && isWriteable(path);
    }

    long Path::getSelfSizeAttr(const String &path)
    {
#ifdef UNICODE
        WS2S_PTR(path, fname);
#else
        const char * fname = path.c_str();
#endif
        struct stat statbuf;
        if(stat(fname, &statbuf) == 0)
            return statbuf.st_size;
        return -1;
    }

    long Path::getSizeAttr(const String &path, FolderSelfSizeOptions options)
    {
        if (isFolder(path)) {
            long size = 0;
            vector<String> files;
            bool ret = listFiles(path, true, true, files);
            for (size_t i = 0; i < files.size(); ++i) {
                size += getSelfSizeAttr(files[i]);
            }
            if (Exclude != options) {
                vector<String> folders;
                ret = ret && listFolders(path, true, folders);
                for (size_t i = 0; i < folders.size(); ++i) {
                    size += getSelfSizeAttr(folders[i]);
                }
            }
            return ret ? size : -1;
        }
        return getSelfSizeAttr(path);
    }

    String Path::getTimeAttr(const String &path, FileTimeOptions options)
    {
#ifdef UNICODE
        WS2S_PTR(path, fname);
#else
        const char * fname = path.c_str();
#endif
        String time;
        struct stat statbuf;
        if(stat(fname, &statbuf) == 0) {
            if (CreateTime == options) {
                time = parseTime(statbuf.st_ctime);
            } else if (LastWriteTime) {
                time = parseTime(statbuf.st_mtime);
            } else if (LastAccessTime) {
                time = parseTime(statbuf.st_atime);
            }
        }
        return time;
    }

    bool Path::isHidden(const String &path)
    {
#if defined(WIN32)
        return (GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_HIDDEN) != 0;
#else
        String fname = getFileName(path);
        return isExist(path) && StringUtils::startsWith(fname, _T("."));
#endif
    }

    bool Path::isReadOnly(const String &path)
    {
#if defined(WIN32)
        return (GetFileAttributes(path.c_str()) & FILE_ATTRIBUTE_READONLY) != 0;
#else
        return isReadable(path) && !isWriteable(path) && !isExecutable(path);
#endif
    }

    bool Path::setHiddenAttr(const String &path, FileHiddenOptions options)
    {
#if defined(WIN32)
        DWORD dw = GetFileAttributes(path.c_str());
        dw = (Hidden == options) ? dw | FILE_ATTRIBUTE_HIDDEN : dw & (~FILE_ATTRIBUTE_HIDDEN);
        return !!SetFileAttributes(path.c_str(), dw);
#else
        String fname = getFileName(path);
        String filename = getDirName(path) + _T(".") + fname;
        return rename(path, filename, true);
#endif
    }

    bool Path::setReadOnlyAttr(const String &path, FileReadOnlyOptions options)
    {
#if defined(WIN32)
        DWORD dw = GetFileAttributes(path.c_str());
        dw = (ReadOnly == options) ? dw | FILE_ATTRIBUTE_READONLY : dw & (~FILE_ATTRIBUTE_READONLY);
        return !!SetFileAttributes(path.c_str(), dw);
#else
        String cmd = (ReadOnly == options)
            _T("sudo chmod +r-w ") + path :
            _T("sudo chmod +r+w ") + path ;
        return !_system(cmd);
#endif
    }

    // 设置文件夹属性， recursionFiles - 是否递归设置所有包含文件
    bool Path::setReadOnlyAttr(const String &path, FileReadOnlyOptions options, bool recursionFiles)
    {
        if (!recursionFiles) {
            return setReadOnlyAttr(path, options);
        }

#if defined(WIN32)
        vector<String> files;
        bool ret = listFiles(path, true, false, files);
        for (size_t i = 0; i < files.size(); ++i) {
            DWORD dw = GetFileAttributes(files[i].c_str());
            dw = (ReadOnly == options) ? dw | FILE_ATTRIBUTE_READONLY : dw & (~FILE_ATTRIBUTE_READONLY);
            SetFileAttributes(files[i].c_str(), dw);
        }
        return ret;
#else
        String cmd = (ReadOnly == options)
            _T("sudo chmod +r-w ") + path :
        _T("sudo chmod +r+w ") + path ;
        return !_system(cmd);
#endif
    }

    bool Path::isFile(const String &path)
    {
#if defined(WIN32)
        return !!_chdir(path); // CreateFile 属性判断
#else
        struct stat st;
        return (lstat(path.c_str(), &st) != -1) && (S_ISREG(st.st_mode)); // 普通文件
#endif
    }

    bool Path::isFolder(const String &path)
    {
#if defined(WIN32)
        return !_chdir(path);
#else
        struct stat st;
        return (lstat(path.c_str(), &st) != -1) && (S_ISDIR(st.st_mode)); // 目录
#endif
    }

    bool Path::mkdir(const String &path)
    {
        if (isExist(path)) {
            return true;
        }

#if defined(WIN32)
        String cmd = _T("mkdir ") + path;
        return !_system(cmd.c_str());
#else
        return !mkdir(path, RW_OK);
#endif
    }

    bool Path::rmdir(const String &path)
    {
        if (!isExist(path)) {
            return true;
        }
        if (isFolder(path)) {
            return deleteDirectory(path);
        }
        return false;
    }

    bool Path::remove(const String &path)
    {
        if (!isExist(path)) {
            return true;
        }
        if (isFile(path)) {
            return !_remove(path);
        }
        return false;
    }

    bool Path::rename(const String &oldname, const String &newname, bool overwrite)
    {
        if (overwrite) {
            if (isFile(oldname) && isFile(newname)) {
                remove(newname);
            } else if (isFolder(oldname) && isFolder(newname)) {
                deleteDirectory(newname);
            }
        }
        return !_rename(oldname, newname);
    }

    // 当前工作目录
    String Path::getCurWorkDir()
    {
        return _getcwd();
    }

    // 获取当前程序绝对路径
    String Path::getAppPath()
    {
#if defined(WIN32)
        TCHAR szPathName[MAX_PATH] = {};
        TCHAR szDriver[MAX_PATH] = {};
        TCHAR szDirectory[MAX_PATH] = {};
        TCHAR szFileName[MAX_PATH] = {};
        TCHAR szExt[MAX_PATH] = {};

        GetModuleFileName(NULL, szPathName, MAX_PATH);
        _tsplitpath_s(szPathName, szDriver, _MAX_DRIVE, szDirectory, _MAX_DIR, szFileName, _MAX_FNAME, szExt, _MAX_EXT);

        //TCHAR szEXEFolder[MAX_PATH];
        //::GetModuleFileName(NULL, szEXEFolder, MAX_PATH);
        //::PathRemoveFileSpec(szEXEFolder);
        return szPathName;
#else
        TCHAR current_absolute_path[MAX_SIZE];
        int cnt = readlink(_T("/proc/self/exe"), current_absolute_path, MAX_SIZE);
        if (cnt < 0 || cnt >= MAX_SIZE)
        {
            return _T("");
        }

        return current_absolute_path;
#endif
        ////获取当前目录绝对路径，即去掉程序名
        //int i;
        //for (i = cnt; i >=0; --i)
        //{
        //    if (current_absolute_path[i] == '/')
        //    {
        //        current_absolute_path[i+1] = '\0';
        //        break;
        //    }
        //}
        //printf("current absolute path:%s\n", current_absolute_path);
    }

    String Path::getDirName(const String &path)
    {
#if defined(WIN32)
        TCHAR separator = '\\';
#else
        TCHAR separator = '/';
#endif
        //// 方法一
        //TCHAR szPathName[MAX_PATH];
        //_tcscpy(szPathName, trim(path).c_str());
        //for (int i = _tcslen(szPathName); i >= 0; --i) {
        //    if (szPathName[i] == separator) {
        //        szPathName[i + 1] = '\0';
        //        break;
        //    }
        //}
        //return szPathName;

        // 方法二
        size_t index = StringUtils::trim(path).find_last_of(separator);
        if (string::npos != index) {
            return path.substr(0, index + 1);
        }
        return path;

        //// 方法三
        //TCHAR szDriver[MAX_PATH] = {};
        //TCHAR szDirectory[MAX_PATH] = {};
        //_tsplitpath(trim(path).c_str(), szDriver, szDirectory, NULL, NULL);
        //if (0 == _tcslen(szDriver)) {
        //    return szDirectory;
        //}
        //return _tcscat(szDriver, szDirectory);
    }

    String Path::getExtName(const String &path)
    {
        TCHAR szExt[MAX_PATH] = {};
        _tsplitpath(StringUtils::trim(path).c_str(), NULL, NULL, NULL, szExt);
        return szExt;
    }

    String Path::getFileName(const String &path)
    {
        TCHAR szFileName[MAX_PATH] = {};
        TCHAR szExt[MAX_PATH] = {};
        _tsplitpath(StringUtils::trim(path).c_str(), NULL, NULL, szFileName, szExt);
        return _tcscat(szFileName, szExt);
    }

    String Path::getFileNameWithoutExtension(const String &path)
    {
        TCHAR szExt[MAX_PATH] = {};
        TCHAR szFileName[MAX_PATH] = {};
        _tsplitpath(StringUtils::trim(path).c_str(), NULL, NULL, szFileName, szExt);
        String fileName = szFileName;
        size_t index = fileName.find_last_not_of(szExt);
        if (string::npos != index) {
            return fileName.substr(0, index);
        }
        return fileName;
    }

    bool Path::hasExtension(const String &path)
    {
        return getExtName(path).length() > 0;
    }
}
