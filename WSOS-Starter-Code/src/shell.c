#include "types.h"
#include "io.h"
#include "fat.h"
#include "multitasking.h"
#include "shell.h"

#define ARGS_SIZE 40

char shellPath[100];

int getFileNameAndExtension(char *name, char *filename, char *ext)
{
	int index = 0;
	char hasExtension = 0;
	while(name[index] != '.' && index < 8)
	{
		filename[index] = name[index];
		index++;

		if (name[index] == '.') hasExtension = 1;
	}

	if (!hasExtension)
	{
		print("Error: file does not have an extension!\n");
		return -1;
	}

	index++;

	int extIndex = 0;
	while(index < 11 && extIndex < 3)
	{
		ext[extIndex] = name[index];
		index++;
		extIndex++;
	}

	// Pad with spaces
	pad(filename, ' ', 8);
	pad(ext, ' ', 3);

	return 0;
}

void getDirectoryName(char *name, char *directoryName)
{
	int index = 0;
	while(index < 8)
	{
		directoryName[index] = name[index];
		index++;
	}

	// Pad with spaces
	pad(directoryName, ' ', 8);

	return;
}

int shellCreateFile(char *name)
{
	// If name is null pointer, there is no file name
	if (name == 0)
	{
		print("Error: no file name specified!\n");
		return -1;
	}

	// Isolate the file name and extension
	char filename[8] = {0};
	char ext[3] = {0};
	if (getFileNameAndExtension(name, filename, ext)) return -1;

	// Search the directory to see if there exists an entry that contains the file name and extension
	char fileDoesNotExist = openFile(filename, ext);

	if (!fileDoesNotExist)
	{
		print("Error: Attempted to create a file that already exists!\n");
		return 1;
	}

	print("Creating File...\n");
	// Create the file on our file system (adds the empty file to our floppy disk)
	createFile(filename, ext);

	return 0;
}

int shellCreateDirectory(char *name)
{
	// If name is a null pointer
	if (name == 0)
	{
		print("Error: no directory name specified!\n");
		return -1;
	}

	// Isolate the name
	char directoryName[8] = {0};
	getDirectoryName(name, directoryName);

	// Search the directory to see if there exists an entry that contains the subdirectory
	char exists = directoryExists(directoryName);

	if (exists)
	{
		print("Error: Attempted to create a directory that already exists!\n");
		return 1;
	}

	print("Creating Subdirectory...\n");
	// Create the subdirectory on our file system (adds the subdirectory to our floppy disk)
	createSubdirectory(directoryName);

	return 0;
}

int shellWriteFile(char *name)
{
	// If name is null pointer, there is no file name
	if (name == 0)
	{
		print("Error: no file name specified!\n");
		return -1;
	}

	// Isolate the file name and extension
	char filename[8] = {0};
	char ext[3] = {0};
	if (getFileNameAndExtension(name, filename, ext)) return -1;

	// Search the directory to see if there exists an entry that contains the file name and extension
	char fileDoesNotExist = openFile(filename, ext);

	if (fileDoesNotExist)
	{
		print("Error: Cannot write to a file that does not exist!\n");
		return 1;
	}

	clearscreen();
	print("Writing File...\n");
	print("Type enter once to add an additional sector to the file.\n");
	print("Type enter twice to close the file.\n");

	uint32 i = 0;
	uint8 prevByte = 0;
	uint8 byte = 0;

	// Let the user type in characters into the file (until they hit ENTER)
	do
	{
		prevByte = byte;
		byte = (uint8)getchar();

		if(byte != '\n')
		{
			// Print the character to the file
			putchar((char)byte);
			writeNextByte(byte);
			i++;
		}
		else if(byte == '\n' && prevByte != '\n')
		{
			clearscreen();
			print("Writing File...\n");
			print("Type enter once to add an additional sector to the file.\n");
			print("Type enter twice to close the file.\n");

			// Fill up the remaining sector with 0's to move onto the next sector
			writeBytes(0, 512 - (i % 512));
			i += 512 - (i % 512);
		}
		
	}while(!(byte == '\n' &&  prevByte == '\n'));
	
	// If we have not overwritten the entire sector, do so now
	// This prevents nasty leftovers in the sector from old writes
	writeBytes(0, 512 - i);
	i += 512 - i;

	// Close the file (save the results to the disk)
	putchar('\n');
	closeFile();

	clearscreen();

	return 0;
}

int shellReadFile(char *name)
{
	// If name is null pointer, there is no file name
	if (name == 0)
	{
		print("Error: no file name specified!\n");
		return -1;
	}

	// Isolate the file name and extension
	char filename[8] = {0};
	char ext[3] = {0};
	if (getFileNameAndExtension(name, filename, ext)) return -1;

	// Search the directory to see if there exists an entry that contains the file name and extension
	char fileDoesNotExist = openFile(filename, ext);

	if (fileDoesNotExist)
	{
		print("Error: Cannot read from a file that does not exist!\n");
		return 1;
	}

	clearscreen();
	print("Reading File...\n");

	// Read one byte from the file
	uint8 byte = readNextByte();

	// Print the contents of the file to the string
	// We are using '-2' as a special code to tell when the last sector ends
	while(byte != (uint8)-2)
	{
		// Print the byte to the screen
		if (byte != 0) putchar((char)byte);

		// Read one byte from the file
		byte = readNextByte();
	}

	// Close the file
	putchar('\n');
	closeFile();

	return 0;
}

int shellDeleteFile(char *name)
{
	// If name is null pointer, there is no file name
	if (name == 0)
	{
		print("Error: no file name specified!\n");
		return -1;
	}

	// Isolate the file name and extension
	char filename[8] = {0};
	char ext[3] = {0};
	if (getFileNameAndExtension(name, filename, ext)) return -1;

	// Search the directory to see if there exists an entry that contains the file name and extension
	char fileDoesNotExist = openFile(filename, ext);

	if (fileDoesNotExist)
	{
		print("Error: Attempted to delete a file that does not exist!\n");
		return 1;
	}

	print("Deleting File...\n");
	deleteFile();

	return 0;
}

int shellDeleteDirectory(char *name)
{
	// If name is null pointer, there is no file name
	if (name == 0)
	{
		print("Error: no file name specified!\n");
		return -1;
	}

	// Isolate the file name and extension
	char directoryName[8] = {0};
	getDirectoryName(name, directoryName);

	// Attempt to delete the subdirectory
	print("Deleting Subdirectory...\n");
	deleteSubdirectory(directoryName);

	return 0;
}

void appendPath(char *directoryName)
{
	// Get path length
	int pathLength = 0;
	while(shellPath[pathLength] != 0){pathLength++;}

	// Get directory name length
	int dirNameLength = 8;
	while(directoryName[dirNameLength - 1] == ' '){dirNameLength--;}

	// If we are navigating to our parent
	if (stringcompare(directoryName, "..", 2))
	{
		int index = pathLength - 1;
		while((shellPath[index] != '/' || pathLength - 1 - index <= 0) && index >= 0)
		{
			shellPath[index] = 0;
			index--;
		}

		return;
	}
	// If we are navigating to ourself, do nothing
	else if (stringcompare(directoryName, ".", 1))
	{
		return;
	}
	else
	{
		for(int i = 0; i < dirNameLength; i++)
		{
			shellPath[pathLength + i + 0] = directoryName[i];
			shellPath[pathLength + i + 1] = '/';
			shellPath[pathLength + i + 2] = 0;
		}

		return;
	}
}

int shellChangeDirectory(char *name)
{
	// If name is null pointer, there is no file name
	if (name == 0)
	{
		print("Error: no file name specified!\n");
		return -1;
	}

	// Isolate the file name and extension
	char directoryName[8] = {0};
	getDirectoryName(name, directoryName);

	// Search the directory to see if there exists an entry that contains the directory
	char exists = directoryExists(directoryName);

	if (!exists)
	{
		print("Error: Attempted to navigate to a subdirectory that does not exist!\n");
		return 1;
	}

	if (!openDirectory(directoryName))
	{
		appendPath(directoryName);
	}

	return 0;
}

void showPath()
{
	// Ask the user to make a selection
	print(shellPath);
	return;
}

void list()
{
	listEntries();
}

void shellHelp()
{
	print("Available Commands: \n");
	print("mkfile *name* - Creates a new file with the specified name.\n");
	print("rmfile *name* - Removes an existing file with the specified name.\n");
	print("mkdir *name* - Creates a new directory with the specified name.\n");
	print("rmfile *name* - Removes an existing directory with the specified name.\n");
	print("read *name* - Displays an existing file's contents with the specified name.\n");
	print("write *name* - Writes to an existing file's contents with the specified name.\n");
	print("cd *name* - Changes the working directory to the directory with the specified name.\n");
	print("ls - Lists the files and directories in the working directory.\n");
	print("help - Displays this information.\n");
	print("quit - Exits the shell process and the operating system halts.\n");

	return;
}

void getArgs(char *args, char **command, char **arg)
{
	scan(args);
	int index = 0;

	*command = args;

	while(args[index] != 0 && index < ARGS_SIZE)
	{
		index++;

		if (args[index] == ' ' && args[index + 1] != 0 && index + 1 < ARGS_SIZE)
		{
			*arg = &args[index + 1];
			break;
		}
	}
}

// The shell processes
void procshell()
{	
	// Initialize the file system
	init_fs();

	print("Type \"help\" to show available commands\n");

	do
	{
		// Show the current path
		showPath();

		// Get user input
		char args[ARGS_SIZE] = {0};
		char *command = args;
		char *arg = 0;
		getArgs(args, &command, &arg);
		putchar('\n');
		
		// Execute based on what they typed
		if 		(stringcompare(command, "mkfile", 6)) shellCreateFile(arg);
		else if	(stringcompare(command, "rmfile", 6)) shellDeleteFile(arg);
		else if	(stringcompare(command, "mkdir",  5)) shellCreateDirectory(arg);
		else if	(stringcompare(command, "rmdir",  5)) shellDeleteDirectory(arg);
		else if	(stringcompare(command, "read",   4)) shellReadFile(arg);
		else if	(stringcompare(command, "write",  5)) shellWriteFile(arg);
		else if (stringcompare(command, "cd",     2)) shellChangeDirectory(arg);
		else if (stringcompare(command, "ls",     2)) list();
		else if (stringcompare(command, "help",   4)) shellHelp();
		else if (stringcompare(command, "quit",   4))
		{
			print("Quitting...\n");
			break;
		}
		else
		{
			// If the input was invalid, just restart loop
			print("Error: Invalid command: ");
			print(command);
			putchar('\n');
		}
		
	}while(1);

	exit();
}

void startshell()
{
	// Create the shell processes
	print("Shell process has started...\n");
	createuserprocess(procshell, (void *) 0x10000);

	// Schedule the next process
	int userprocs = ready_process_count();

	// As long as we have ready user processes to run
	while(userprocs > 0)
	{
		// Yield to them
		yield();
		userprocs = ready_process_count();
	}

    print("\n\nShell process has exited...\n");
    exit();
}