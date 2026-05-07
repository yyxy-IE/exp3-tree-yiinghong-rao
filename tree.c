/**
 * 实验：目录树查看器（仿 Linux tree 命令）
 * 学号：__________  姓名：__________
 * 说明：已补全所有标记为 TODO 的函数体
 * 目录树查看器（仿 Linux tree 命令）
 * 完整实现版本（C语言，左孩子右兄弟二叉树）
 * 编译：gcc -o tree tree.c -std=c99
 * 运行：./tree  或  ./tree 目标目录路径
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// ================== 二叉树结点定义 ==================
typedef struct FileNode {
    char *name;                  // 文件/目录名
    int isDir;                   // 1:目录 0:文件
    struct FileNode *firstChild; // 左孩子：第一个子项
    struct FileNode *nextSibling;// 右兄弟：下一个同层项
} FileNode;

// ================== 函数声明 ==================
FileNode* createNode(const char *name, int isDir);
int cmpNode(const void *a, const void *b);
FileNode* buildTree(const char *path);
void printTree(FileNode *node, const char *prefix, int isLast);
int countNodes(FileNode *root);
int countLeaves(FileNode *root);
int treeHeight(FileNode *root);
void countDirFile(FileNode *root, int *dirs, int *files);
void freeTree(FileNode *root);
char* getBaseName(void);

// ================== 已补全的函数 ==================

// 创建新结点（分配内存、复制字符串、初始化指针）
FileNode* createNode(const char *name, int isDir) {
    FileNode *node = (FileNode*)malloc(sizeof(FileNode));
    if (!node) return NULL;
    
    // 复制文件名
    node->name = (char*)malloc(strlen(name) + 1);
    if (!node->name) {
        free(node);
        return NULL;
    }
    strcpy(node->name, name);
    
    node->isDir = isDir;
    node->firstChild = NULL;
    node->nextSibling = NULL;
    return node;
}

// 比较函数，用于 qsort 对子项按名称排序
int cmpNode(const void *a, const void *b) {
    FileNode *nodeA = *(FileNode**)a;
    FileNode *nodeB = *(FileNode**)b;
    // 字典序排序
    return strcmp(nodeA->name, nodeB->name);
}

// 递归构建目录树（核心难点）
FileNode* buildTree(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) return NULL;

    // 提取目录名
    const char *lastSlash = strrchr(path, '/');
    char *nodeName;
    if (lastSlash == NULL || (lastSlash == path && strlen(path) == 1)) {
        // 根目录 / 或无路径分隔符
        nodeName = (char*)malloc(2);
        strcpy(nodeName, lastSlash ? "/" : path);
    } else {
        nodeName = (char*)malloc(strlen(lastSlash + 1) + 1);
        strcpy(nodeName, lastSlash + 1);
    }

    FileNode *root = createNode(nodeName, 1);
    free(nodeName);
    if (!root) {
        closedir(dir);
        return NULL;
    }

    // 临时数组存储子节点
    FileNode **children = NULL;
    int childCount = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // 拼接完整路径
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(fullPath, &st) != 0) continue;

        FileNode *child;
        if (S_ISDIR(st.st_mode)) {
            // 递归构建子目录
            child = buildTree(fullPath);
        } else {
            // 创建文件节点
            child = createNode(entry->d_name, 0);
        }

        if (child) {
            children = (FileNode**)realloc(children, (childCount + 1) * sizeof(FileNode*));
            children[childCount++] = child;
        }
    }

    closedir(dir);

    // 排序子节点
    if (childCount > 0) {
        qsort(children, childCount, sizeof(FileNode*), cmpNode);

        // 链接成兄弟链表
        root->firstChild = children[0];
        for (int i = 0; i < childCount - 1; i++) {
            children[i]->nextSibling = children[i + 1];
        }
    }

    free(children);
    return root;
}

// 树形输出（仿 tree 命令）
void printTree(FileNode *node, const char *prefix, int isLast) {
    if (!node) return;

    // 打印当前节点
    printf("%s%s%s", prefix, isLast ? "`-- " : "|-- ", node->name);
    if (node->isDir) printf("/");
    printf("\n");

    // 递归打印子节点
    if (node->firstChild) {
        char newPrefix[1024];
        // 拼接新前缀
        snprintf(newPrefix, sizeof(newPrefix), "%s%s", prefix, isLast ? "    " : "|   ");

        FileNode *child = node->firstChild;
        int count = 0;
        FileNode *tmp = child;
        while (tmp) { count++; tmp = tmp->nextSibling; }

        int idx = 0;
        while (child) {
            printTree(child, newPrefix, ++idx == count);
            child = child->nextSibling;
        }
    }
}

// 统计二叉树结点总数
int countNodes(FileNode *root) {
    if (!root) return 0;
    // 当前节点 + 子节点 + 兄弟节点
    return 1 + countNodes(root->firstChild) + countNodes(root->nextSibling);
}

// 统计叶子结点数（firstChild == NULL 的结点）
int countLeaves(FileNode *root) {
    if (!root) return 0;
    // 无孩子 = 叶子节点
    if (!root->firstChild)
        return 1 + countLeaves(root->nextSibling);
    // 有孩子 = 递归统计子树
    return countLeaves(root->firstChild) + countLeaves(root->nextSibling);
}

// 计算二叉树高度（根深度为1，空树高度为0）
int treeHeight(FileNode *root) {
    if (!root) return 0;
    int childH = treeHeight(root->firstChild) + 1;
    int siblingH = treeHeight(root->nextSibling);
    // 取子树高度和兄弟树高度的最大值
    return (childH > siblingH) ? childH : siblingH;
}

// 统计目录数和文件数（遍历整棵树）
void countDirFile(FileNode *root, int *dirs, int *files) {
    if (!root) return;
    if (root->isDir) (*dirs)++;
    else (*files)++;
    // 递归遍历子节点和兄弟节点
    countDirFile(root->firstChild, dirs, files);
    countDirFile(root->nextSibling, dirs, files);
}

// 释放整棵树的内存
void freeTree(FileNode *root) {
    if (!root) return;
    // 后序释放：先子节点，再兄弟节点，最后自身
    freeTree(root->firstChild);
    freeTree(root->nextSibling);
    free(root->name);
    free(root);
}

// 获取当前工作目录的“基本名称”（用于显示根结点名）
char* getBaseName(void) {
    char *cwd = getcwd(NULL, 0);
    if (!cwd) return NULL;

    char *lastSlash = strrchr(cwd, '/');
    char *baseName;

    if (lastSlash == NULL || (lastSlash == cwd && strlen(cwd) == 1)) {
        baseName = (char*)malloc(2);
        strcpy(baseName, lastSlash ? "/" : cwd);
    } else {
        baseName = (char*)malloc(strlen(lastSlash + 1) + 1);
        strcpy(baseName, lastSlash + 1);
    }

    free(cwd);
    return baseName;
}

int main(int argc, char *argv[]) {
    char targetPath[1024];
    if (argc >= 2) {
        strncpy(targetPath, argv[1], sizeof(targetPath)-1);
        targetPath[sizeof(targetPath)-1] = '\0';
    } else {
        if (getcwd(targetPath, sizeof(targetPath)) == NULL) {
            perror("getcwd");
            return 1;
        }
    }

    int len = strlen(targetPath);
    if (len > 0 && targetPath[len-1] == '/')
        targetPath[len-1] = '\0';

    struct stat st;
    if (stat(targetPath, &st) != 0) {
        perror("stat");
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "错误: %s 不是目录\n", targetPath);
        return 1;
    }

    FileNode *root = buildTree(targetPath);
    if (!root) {
        fprintf(stderr, "无法构建目录树\n");
        return 1;
    }

    // 输出根目录名
    char *displayName = NULL;
    if (argc >= 2) {
        displayName = root->name;
    } else {
        displayName = getBaseName();
    }
    printf("%s/\n", displayName);
    if (argc < 2) free(displayName);

    FileNode *child = root->firstChild;
    int childCount = 0;
    FileNode *tmp = child;
    while (tmp) { childCount++; tmp = tmp->nextSibling; }
    int idx = 0;
    while (child) {
        int isLast = (++idx == childCount);
        printTree(child, "", isLast);
        child = child->nextSibling;
    }

    int dirs = 0, files = 0;
    countDirFile(root, &dirs, &files);
    printf("\n%d 个目录, %d 个文件\n", dirs, files);
    printf("二叉树结点总数: %d\n", countNodes(root));
    printf("叶子结点数: %d\n", countLeaves(root));
    printf("树的高度: %d\n", treeHeight(root));

    freeTree(root);
    return 0;
}