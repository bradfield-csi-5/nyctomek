#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/param.h> // #define MAX
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <math.h>

// File names can be up to 1024 characters, including
// the null terminator.  From `man dirent`.
#define MAX_FILE_NAME_LEN 1024
#define FD_STDOUT 1

enum OptionalFlags
{
	DASH_1 = 0x00000001, // ls -1    (The numeric digit "one".)  Force output to be one entry per line.
	                     //          This is the default when output is not to a terminal.

	DASH_A = 0x00000002, // ls -A    List all entries except for . and ...  Always set for super-user.

	DASH_a = 0x00000004, // ls -a    Include directory entries whose names begin with a dot (.).

	DASH_d = 0x00000008, // ls -d    Directories are listed as plain files (not searched recursively).

	DASH_l = 0x00000010, // ls -l    (The lowercase letter "ell".)  List in long format.
	                     //          If the output is to a terminal, a total sum for all the sizes is
	                     //          output on a line before the long listing.

	DASH_R = 0x00000020, // ls -R    Recursively list subdirectories encountered.

	DASH_r = 0x00000040, // ls -r    Reverse the order of the sort to get reverse lexicographical order or 
                         //       	 the oldest entries first (or largest files last, if combined with sort by size).

	DASH_S = 0x00000080, // ls -S 	 Sort files by size.

	DASH_t = 0x00000100, // ls -t    Sort by time modified (most recently modified first) before sorting
	                     //          the operands by lexographical order.

	DASH_T = 0x00000200, // ls -T    When used with the -l (lowercase letter ``ell'') option, display complete time
						 // 		 information for the file, including month, day, hour, minute, second, and year.

	DASH_C = 0x00000400, // ls -C	 Force multi-column output; this is the default when output is to a terminal.
};

typedef struct FileInfo
{
	char filename[MAX_FILE_NAME_LEN];
	struct stat statInfo;
} 
FileInfo_t;

typedef struct FileInfoListNode
{
	FileInfo_t *fileInfo;
	struct FileInfoListNode *next;
}
FileInfoListNode_t;


FileInfo_t* getFileInfo(const char *filename)
{
	struct stat statInfo = {0};
	if(lstat(filename, &statInfo))
	{
		static char prefix[MAX_FILE_NAME_LEN + 5]; // "ls: " + filename + NULL.
		sprintf(prefix, "ls: %s", filename);
		perror(prefix);
		return NULL;
	}

	FileInfo_t *fileInfo = calloc(1, sizeof(FileInfo_t));
	strncpy(fileInfo->filename, filename, sizeof fileInfo->filename);
	memmove(&fileInfo->statInfo, &statInfo, sizeof fileInfo->statInfo);
	return fileInfo;
}


void addFileInfoToList(FileInfo_t *fileInfo, FileInfoListNode_t **head, FileInfoListNode_t **tail)
{

	FileInfoListNode_t *element = calloc(1, sizeof(FileInfoListNode_t));
	element->fileInfo  = fileInfo;
	if(*head == NULL)
	{
		*head = element;
	}
	if(*tail != NULL)
	{
		(*tail)->next = element;
	}
	*tail = element;
}


void freeFileInfoList(FileInfoListNode_t *list)
{
	while(list)
	{
		FileInfoListNode_t *next = list->next;
		free(list->fileInfo);
		free(list);
		list = next;
	}
}


void getInfoForFilenames(
	// input
	char *filenames[],
	int filenameCount,
	int options, 
	// output
	FileInfoListNode_t **regularFileList,
	size_t *regularFileCount,
	FileInfoListNode_t **directoryList,
	size_t *directoryCount)
{
	FileInfoListNode_t *regularFileTail = NULL, *directoryTail = NULL;

	while(filenameCount-- > 0)
	{
		FileInfo_t *fileInfo = getFileInfo(*filenames++);
		if(fileInfo)
		{
			if((fileInfo->statInfo.st_mode & S_IFMT) == S_IFDIR && !(options & DASH_d))
			{
				addFileInfoToList(fileInfo, directoryList, &directoryTail);
				(*directoryCount)++;
			}
			else
			{
				addFileInfoToList(fileInfo, regularFileList, &regularFileTail);
				(*regularFileCount)++;
			}
		}
	}
}


char *getFileNameWithoutPath(char *filename, char *relativePath)
{
	if(strstr(filename, relativePath) == filename)
	{
		return filename + strlen(relativePath);
	}
	return filename;
}


int compareLexographically(const void *lhs, const void *rhs)
{
	const FileInfo_t **l = (const FileInfo_t **)(lhs);
	const FileInfo_t **r = (const FileInfo_t **)(rhs);
	return strcmp((**l).filename, (**r).filename);
}

int compareReverseLexographically(const void *lhs, const void *rhs)
{
	return compareLexographically(lhs, rhs) * -1;
}

int valcmp(int lhs, int rhs)
{
	if(lhs < rhs)
	{
		return -1;
	}
	if(lhs > rhs)
	{
		return 1;
	}
	return 0;
}


int compareByFileSize(const void *lhs, const void *rhs)
{
	const FileInfo_t **l = (const FileInfo_t **)(lhs);
	const FileInfo_t **r = (const FileInfo_t **)(rhs);
	int result = valcmp((**l).statInfo.st_size, (**r).statInfo.st_size);
	// Largest files first.
	return result * -1;
}


int compareByReverseFileSize(const void *lhs, const void *rhs)
{
	return compareByFileSize(lhs, rhs) * -1;
}


int compareByTimeModified(const void *lhs, const void *rhs)
{
	const FileInfo_t **l = (const FileInfo_t **)(lhs);
	const FileInfo_t **r = (const FileInfo_t **)(rhs);

	int result = valcmp((**l).statInfo.st_mtimespec.tv_sec, (**r).statInfo.st_mtimespec.tv_sec);

	// Most recently modified files (greatest timestamp) first.
	result *= -1; 
	if(0 == result)
	{
		result = compareLexographically(lhs, rhs);
	}
	return result;
}


int compareByReverseTimeModified(const void *lhs, const void *rhs)
{
	return compareByTimeModified(lhs, rhs) * -1;
}


FileInfo_t** sortFileInfo(FileInfoListNode_t *fileInfoNode, size_t fileInfoCount, int options)
{
	FileInfo_t **fileInfoSorted = calloc(fileInfoCount, sizeof(FileInfo_t*));
	size_t i = 0;
	while(fileInfoNode)
	{
		fileInfoSorted[i++] = fileInfoNode->fileInfo;
		fileInfoNode = fileInfoNode->next;
	}

	int (*compare)(const void *, const void *) = compareLexographically;
	if(options & DASH_t)
	{
		compare = compareByTimeModified;
	}
	if(options & DASH_S)
	{
		compare = compareByFileSize;
	}
	if(options & DASH_r)
	{
		if(compare == compareByFileSize)
		{
			compare = compareByReverseFileSize;
		}
		if(compare == compareByTimeModified)
		{
			compare = compareByReverseTimeModified;
		}
		if(compare == compareLexographically)
		{
			compare = compareReverseLexographically;
		}
	}

	qsort(fileInfoSorted, fileInfoCount, sizeof(FileInfo_t*), compare);
	return fileInfoSorted;
}


void listFilesMultiColumnOutput(FileInfo_t **fileInfo, size_t count, char *relativePath)
{
	if(count == 0)
		return;

	int terminalWidth = 0;
	char *columnsEnvVar = getenv("COLUMNS");
	//printf("listFilesMultiColumnOutput(): columnsEnvVar: %s.\n", columnsEnvVar);
	if(columnsEnvVar)
	{
		terminalWidth = atoi(columnsEnvVar);
	}
	else
	{

		struct winsize w;
		if(ioctl(0, TIOCGWINSZ, &w) != -1)
		{
			terminalWidth = w.ws_col;
		}
	}
	//printf("listFilesMultiColumnOutput(): terminalWidth: %d.\n", terminalWidth);
	if(terminalWidth)
	{
		size_t maxFileNameLen = 0;
		for(size_t i = 0; i < count; ++i)
		{
			char *displayFileName = getFileNameWithoutPath(fileInfo[i]->filename, relativePath);
			size_t fileNameLen =  strlen(displayFileName);
			maxFileNameLen = MAX(fileNameLen, maxFileNameLen);
		}

		const int COLUMN_PADDING = 5;
		int columnWidth = maxFileNameLen + COLUMN_PADDING;
		int numColumns = terminalWidth / columnWidth;

		int numRows = count / numColumns;
		if(count % numColumns) numRows++;

		for(size_t row = 0; row < numRows; row++)
		{
			for(size_t column = 0; column < numColumns; column++)
			{
				size_t i = column * (numRows-1) + column + row;
				if(i < count)
				{
					char *displayFileName = getFileNameWithoutPath(fileInfo[i]->filename, relativePath);
					printf("%-*s", columnWidth, displayFileName);
				}
			}
			printf("\n");
		}
	}
	else
	{
		// Unable to print in multi column mode.
		for(size_t i = 0; i < count; i++)
		{
			printf("%s\n", fileInfo[i]->filename);
		}
	}
	
}


void formatFileMode(const struct stat *statInfo, char *fileMode)
{
	switch(statInfo->st_mode & S_IFMT)
	{
		case S_IFIFO:  fileMode[0] = 'p'; break;
		case S_IFCHR:  fileMode[0] = 'c'; break;
		case S_IFDIR:  fileMode[0] = 'd'; break;
		case S_IFBLK:  fileMode[0] = 'b'; break;
		case S_IFLNK:  fileMode[0] = 'l'; break;
		case S_IFSOCK: fileMode[0] = 's'; break;
		case S_IFREG:  fileMode[0] = '-'; break;
		default:       fileMode[0] = ' '; break;
	}
	fileMode[1] = statInfo->st_mode & S_IRUSR ? 'r' : '-';
	fileMode[2] = statInfo->st_mode & S_IWUSR ? 'w' : '-';
	fileMode[3] = statInfo->st_mode & S_IXUSR ? 'x' : '-';
	fileMode[4] = statInfo->st_mode & S_IRGRP ? 'r' : '-';
	fileMode[5] = statInfo->st_mode & S_IWGRP ? 'w' : '-';
	fileMode[6] = statInfo->st_mode & S_IXGRP ? 'x' : '-';
	fileMode[7] = statInfo->st_mode & S_IROTH ? 'r' : '-';
	fileMode[8] = statInfo->st_mode & S_IWOTH ? 'w' : '-';
	fileMode[9] = statInfo->st_mode & S_IXOTH ? 'x' : '-';

	// Unimplemented for now.
	fileMode[10] = ' ';
}


void listFilesLongOutput(FileInfo_t **fileInfo, size_t count, int options, char *relativePath)
{
	// Compute column widths.
	size_t maxNumLinks = 0, maxFileSize = 0, maxUserNameLen = 0, maxGroupNameLen = 0, blockCount = 0;
	for(size_t index = 0; index < count; index++)
	{
		FileInfo_t *element = fileInfo[index];

		uid_t userId    = element->statInfo.st_uid;
		gid_t groupId   = element->statInfo.st_gid;
		maxNumLinks     = MAX(maxNumLinks, element->statInfo.st_nlink);
		maxFileSize     = MAX(maxFileSize, element->statInfo.st_size);
		maxUserNameLen  = MAX(maxUserNameLen, strlen(getpwuid(userId)->pw_name));
		maxGroupNameLen = MAX(maxGroupNameLen, strlen(getgrgid(groupId)->gr_name));

		blockCount += element->statInfo.st_blocks;
	}

	int numLinksWidth = maxNumLinks ? (int)(log10((double)maxNumLinks)) + 1 : 1;
	int fileSizeWidth = maxFileSize ? (int)(log10((double)maxFileSize)) + 1 : 1;

	printf("total %lu\n", blockCount);

	while(count > 0)
	{
		char fileMode[12] = {0};
		formatFileMode(&(*fileInfo)->statInfo, fileMode);
		printf("%-*s", (int)(sizeof fileMode), fileMode);

		printf("%*d ", numLinksWidth, (*fileInfo)->statInfo.st_nlink);

		uid_t userId   = (*fileInfo)->statInfo.st_uid;
		char *userName = getpwuid(userId)->pw_name;
		printf("%-*s", (int)maxUserNameLen + 2, userName);

		gid_t groupId   = (*fileInfo)->statInfo.st_gid;
		char *groupName = getgrgid(groupId)->gr_name;
		printf("%-*s", (int)maxGroupNameLen + 2, groupName);

		printf("%*lld ", (int)fileSizeWidth, (*fileInfo)->statInfo.st_size);

		char timeModifiedBuffer[128] = {0};

		if(options & DASH_T)
		{
			strftime(timeModifiedBuffer,
					 sizeof timeModifiedBuffer,
					 "%b %e %T %Y",
					 localtime(&((*fileInfo)->statInfo.st_mtimespec.tv_sec)));
		}
		else
		{
			strftime(timeModifiedBuffer, 
					  sizeof timeModifiedBuffer,
					  "%b %e %R",
					  localtime(&((*fileInfo)->statInfo.st_mtimespec.tv_sec)));

		}
		printf("%s ", timeModifiedBuffer);


		char *displayFileName = getFileNameWithoutPath((*fileInfo)->filename, relativePath);
		printf("%s\n", displayFileName);
		count--;
		fileInfo++;
	}
}


void listFilesOneEntryPerLine(FileInfo_t **fileInfo, size_t count, char *relativePath)
{
	while(count > 0)
	{
		char *displayFileName = getFileNameWithoutPath((*fileInfo)->filename, relativePath);
		printf("%s\n", displayFileName);
		count--;
		fileInfo++;
	}
}


void listFileInfo(FileInfo_t **fileInfo, size_t count, int options, char *relativePath)
{
	if(options & DASH_l)
	{
		listFilesLongOutput(fileInfo, count, options, relativePath);
	}
	else if(options & DASH_1)
	{
		listFilesOneEntryPerLine(fileInfo, count, relativePath);
	}
	else if(options & DASH_C || isatty(FD_STDOUT))
	{
		listFilesMultiColumnOutput(fileInfo, count, relativePath);
	}
}


void listDirectoryContents(FileInfo_t *directoryInfo, int printHeaderRow, int options)
{
	size_t fileInfoCount = 0;
	
	FileInfoListNode_t *head = NULL, *tail = NULL;

	DIR *dirPtr = opendir(directoryInfo->filename);
	struct dirent *dirEntry = NULL;
	while((dirEntry = readdir(dirPtr)))
	{
		if(0 == strcmp(".",  dirEntry->d_name) ||
		   0 == strcmp("..", dirEntry->d_name))
		{
			if(!(options & DASH_a))
			{
				continue;
			}
		}
		if(*(dirEntry->d_name) == '.')
		{
			if(!(options & DASH_A) && !(options & DASH_a))
			{
				continue;
			}
		}
		char filename[MAX_FILE_NAME_LEN] = {0};
		snprintf(filename, sizeof filename, "%s/%s", directoryInfo->filename, dirEntry->d_name);
		FileInfo_t *fileInfo = getFileInfo(filename);
		if(fileInfo)
		{
			addFileInfoToList(fileInfo, &head, &tail);
			fileInfoCount++;
		}
	}
	closedir(dirPtr);
	FileInfo_t **sortedFileInfo = sortFileInfo(head, fileInfoCount, options);

	if(printHeaderRow)
	{
		printf("%s:\n", directoryInfo->filename);
	}

	char relativePath[MAX_FILE_NAME_LEN] = {0};
	snprintf(relativePath, sizeof(relativePath), "%s/", directoryInfo->filename);
	listFileInfo(sortedFileInfo, fileInfoCount, options, relativePath);
	if(options & DASH_R)
	{
		for(size_t i = 0; i < fileInfoCount; i++)
		{
			if(((sortedFileInfo[i])->statInfo.st_mode & S_IFMT) == S_IFDIR)
			{
				listDirectoryContents(sortedFileInfo[i], 1, options);
			}
		}
	}
	free(sortedFileInfo);
	freeFileInfoList(head);
}


void listDirectoryInfo(FileInfo_t **directoryInfo, size_t directoryInfoCount, int printHeaderRow, int options)
{
	while(directoryInfoCount > 0)
	{
		listDirectoryContents(*directoryInfo, printHeaderRow, options);
		directoryInfo++;
		directoryInfoCount--;
	}
}


int parseOptionalArguments(int *argc, char **argv[])
{
	int flags = 0;
	while(*argc)
	{
		char *arg = **argv;
		if(arg[0] == '-' && arg[1] != '\0' && !isspace(arg[1]))
		{
			while(*(++arg))
			{
				switch(*arg)
				{
				case '1':
					flags |= DASH_1;
					break;
				case 'A':
					flags |= DASH_A;
					break;
				case 'a':
					flags |= DASH_a;
					break;
				case 'C':
					flags |= DASH_C;
					break;
				case 'd':
					flags |= DASH_d;
					break;
				case 'l':
					flags |= DASH_l;
					break;
				case 'R':
					flags |= DASH_R;
					break;
				case 'r':
					flags |= DASH_r;
					break;
				case 'S':
					flags |= DASH_S;
					break;
				case 't':
					flags |= DASH_t;
					break;
				case 'T':
					flags |= DASH_T;
					break;
				default:
					printf("Unknown option specified -- %c.\n", *arg);
					break;
				}
			}
			--*argc;
			++*argv;
		}
		else
		{
			// We've processed all optional arguments.
			break;
		}
	}
	return flags;
}


void processLSCommand(int argc, char *argv[])
{
	char *defaultArgs[] = { "." };
	int options = parseOptionalArguments(&argc, &argv);

	char **filenames = argv;
	int numFilenames = argc;

	if(numFilenames == 0)
	{
		filenames = defaultArgs;
		numFilenames = 1;
	}

	FileInfoListNode_t *regularFileList = NULL, *directoryList = NULL;
	size_t numRegularFiles = 0, numDirectories = 0;

	getInfoForFilenames(
		filenames, numFilenames, options,
		&regularFileList, &numRegularFiles,
		&directoryList, &numDirectories);

	if(numRegularFiles)
	{
		FileInfo_t **sortedFileInfo = sortFileInfo(regularFileList, numRegularFiles, options);
		listFileInfo(sortedFileInfo, numRegularFiles, options, "");
		free(sortedFileInfo);
	}

	if(numDirectories)
	{
		FileInfo_t **sortedDirectoryInfo = sortFileInfo(directoryList, numDirectories, options);
		int printHeaderRow = numRegularFiles + numDirectories > 1;
		listDirectoryInfo(sortedDirectoryInfo, numDirectories, printHeaderRow, options);
		free(sortedDirectoryInfo);
	}

	freeFileInfoList(regularFileList);
	freeFileInfoList(directoryList);
}


int main(int argc, char *argv[])
{
	processLSCommand(argc-1, argv+1);
	return 0;
}

