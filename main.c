#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAXLEN 1000
#define MAXFILES 500
#define MAXCAT 10
#define MAXSTRLEN 100


typedef struct filedata{
	char* fullname;
	char* filename;
	char* filetype;
	int depth;
}FILEDATA;

typedef struct catalog{
	char* name;
	int depth;
	struct catalog* next;
}CATALOG;

FILEDATA* arr;
CATALOG* catarr;
char* currentloc;

int is_regular_file(const char *path)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    if (d)
    {
        closedir(d);
        return 0;
    }
    return 1;
}

void init()
{
	arr = (FILEDATA*)malloc(sizeof(FILEDATA) * MAXFILES);
	catarr = (CATALOG*)malloc(sizeof(CATALOG) * MAXCAT);
	int i;
	for(i = 0; i < MAXFILES;i++)
	{
		arr[i].fullname = NULL;
		arr[i].filename = NULL;
		arr[i].filetype = NULL;
		arr[i].depth = 0;
	}
	for(i = 0; i < MAXCAT;i++)
	{
		catarr[i].name = NULL;
		catarr[i].depth = 0;
		catarr[i].next = (CATALOG*)NULL;
	}
}

void insert(char* fullname,char* filename, char* filetype, int depth, int loc)
{
	if(arr[loc].filename == (char*)NULL)
	{
		arr[loc].fullname = (char*)malloc(sizeof(char)*strlen(fullname));
		arr[loc].filename = (char*)malloc(sizeof(char)*strlen(filename));
		arr[loc].filetype = (char*)malloc(sizeof(char)*strlen(filetype));
		strcpy(arr[loc].fullname, fullname);
		strcpy(arr[loc].filename, filename);
		strcpy(arr[loc].filetype, filetype);
		arr[loc].depth = depth;
		return;
	}
	else
	{
		int startloc = loc++;
		while(arr[loc].filename != (char*)NULL)
		{
			loc++;
			if(loc == MAXFILES)
				loc = 0;
			if(loc == startloc)
			{
				printf("BRAK MIEJSCA NA WIEKSZA ILOSC DANYCH\n");
				return;
			}
		}
		arr[loc].fullname = (char*)malloc(sizeof(char)*strlen(fullname));
		arr[loc].filename = (char*)malloc(sizeof(char)*strlen(filename));
		arr[loc].filetype = (char*)malloc(sizeof(char)*strlen(filetype));
		strcpy(arr[loc].fullname, fullname);
		strcpy(arr[loc].filename, filename);
		strcpy(arr[loc].filetype, filetype);
		arr[loc].depth = depth;
	}
}

void deletefromtab(int loc)
{
	arr[loc].fullname = NULL;
	arr[loc].filename = NULL;
	arr[loc].filetype = NULL;
	arr[loc].depth = 0;
}

void addtocatarr(char* name, int depth)
{
	
	int i,loc;
	for(i = 0; i < strlen(name); i++)
		loc += name[i];
	loc %= MAXCAT;
	if(catarr[loc].name == (char*)NULL)
	{
		
		catarr[loc].name = (char*)malloc(sizeof(char)*strlen(name));
		strcpy(catarr[loc].name, name);
		catarr[loc].depth = depth;
		catarr[loc].next = (CATALOG*)NULL;
		return;
	}
	else
	{
		
		CATALOG* temp;
		temp = catarr+loc;
		while(temp->next != (CATALOG*)NULL)
		{
			temp = temp->next;
		}
		CATALOG* next = (CATALOG*)malloc(sizeof(CATALOG));
		next->name = (char*)malloc(sizeof(char)*strlen(name));
		strcpy(next->name, name);
		next->depth = depth;
		next->next = (CATALOG*)NULL;
		temp->next = next;
	}
}

void addtotab(char* name, int depth)
{
	int len = strlen(name);
	int ftypelen = 0,fnamelen;
	int sum = 0;
	int dotindex;
	int i;
	for(i = len - 1;i>=0;i--)
	{
		if(name[i] == '.')
		{
			sum+=name[i];
			dotindex = i;
			break;
		}
		else
		{
			ftypelen++;
			sum+=name[i];
		}
	}
	if(ftypelen == len)
	{
		insert(name,name,"none",0,sum % MAXFILES);
		return;
	}
	i++;
	fnamelen = dotindex;
	char* filetype = (char*)malloc(sizeof(char)*ftypelen);
	char* filename = (char*)malloc(sizeof(char)*fnamelen);
	memcpy(filename,name,fnamelen);
	memcpy(filetype,name+dotindex+1,ftypelen);
	filetype[ftypelen] = '\0';
	filename[fnamelen] = '\0';
	int index = sum % MAXFILES;
	insert(name, filename, filetype, depth, index);
}

int addfile(char* name)
{
	int check = searchforfile(name);
	int i = 1;
	while(check != 0)
	{
		char c[10];
		itoa(i, c, 10);
		char* temp = (char*)malloc(sizeof(char) * (strlen(name)+strlen(c)));
		strcat(temp, c);
		strcat(temp, name);
		check = searchforfile(temp);
		if(check == 0)
		{
			strcpy(name, temp);
		}
		else
		{
			memset(c,0,strlen(c));
			i++;
		}
	}
	FILE* fn = fopen(name, "w");
	if(fn == NULL)
	{
		return 0;
	}
	fclose(fn);
	addtotab(name, 0);
	return 1;
}

int removefile(char* name)//return 0 if remove succesfully
{
	int loc = searchforfile(name);
	if(arr[loc].depth != 0)
		return 1;
	deletefromtab(loc);
	return remove(name);
}

void printdirwithsubdirs(char* name,int depth)
{
	DIR *d;
    struct dirent *dir;
    d = opendir(name);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
        	if(strcmp(dir->d_name,".") && strcmp(dir->d_name,".."))
        	{
        		char* path = (char*)malloc(sizeof(char)*(strlen(name)+1+strlen(dir->d_name)));
        		strcpy(path,name);
        		strcat(path,"/");
        		strcat(path,dir->d_name);
        		if(is_regular_file(path))
        		{
        			printf("%*s", depth*4, "");
        			printf("%s\n", dir->d_name);
        			addtotab(dir->d_name,depth);
				}
        		else
				{
					printf("%*s", depth*4, "");
					printf("Katalog %s:\n", dir->d_name);
					addtocatarr(dir->d_name, depth);
					printdirwithsubdirs(path,depth+1);	
				}	
			}
        }
        closedir(d);
    }
}

void printdir(char* name,int depth)
{
	init();
	DIR *d;
    struct dirent *dir;
    d = opendir(name);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
        	if(strcmp(dir->d_name,".") && strcmp(dir->d_name,".."))
        	{
        		char* path = (char*)malloc(sizeof(char)*(strlen(name)+1+strlen(dir->d_name)));
        		strcpy(path,name);
        		strcat(path,"/");
        		strcat(path,dir->d_name);
        		if(is_regular_file(path))
        		{
        			printf("%*s", depth, "");
        			printf("%s\n", dir->d_name);
        			addtotab(dir->d_name,depth);
				}
        		else
				{
					printf("%*s", depth, "");
					printf("%s (KATALOG)\n", dir->d_name);
					addtocatarr(dir->d_name, depth);	
				}	
			}
        }
        closedir(d);
    }
}

int searchforfile(char* fname)
{
	int i;
	for(i = 0; i < MAXFILES; i++)
	{
		if(arr[i].fullname != (char*)NULL && strcmp(arr[i].fullname, fname) == 0)
		{
			return 1;
		}
	}
	return 0; 
}

void searchforfile_partname(char* pname)
{
	int len = strlen(pname);
	int i;
	for(i = 1; i < MAXFILES; i++)
	{
		int check = 1;
		int j;
		if(arr[i].fullname == (char*)NULL)
			continue;
		if(strncmp(pname, arr[i].fullname, len) == 0)
			printf("%s %d\n", arr[i].fullname, arr[i].depth);
	} 
}

void searchforfile_type(char* type)
{
	int i;
	for(i = 1; i < MAXFILES; i++)
	{
		if(arr[i].fullname != (char*)NULL && strcmp(arr[i].filetype, type) == 0)
		{
			printf("%s %d\n", arr[i].fullname, arr[i].depth);
		}
	} 
}

int searchforfolder(char* name)
{
	int i;
	for(i = 0; i < MAXCAT; i++)
	{
		if(catarr[i].name != (char*)NULL)
		{
			if(strcmp(catarr[i].name, name) == 0)
					return 1;
			CATALOG* temp = catarr+i;
			while(temp->next != (CATALOG*)NULL)
			{
				temp = temp->next;
				if(strcmp(temp->name, name) == 0)
					return 1;
			}
		}
	} 
	return 0;
}

void printarr()
{
	int i;
	for(i = 1; i < MAXFILES;i++)
		if(arr[i].filename != (char*)NULL)
			printf("%d %s . %s %d\n",i,arr[i].filename,arr[i].filetype,arr[i].depth);	
}

void printcatarr()
{
	int i;
	for(i = 1; i < MAXCAT;i++)
		if(catarr[i].name != (char*)NULL)
		{
			printf("%d %s %d\n",i,catarr[i].name,catarr[i].depth);	
			CATALOG* temp= catarr+i;
			while(temp->next != (CATALOG*)NULL)
			{
				temp = temp->next;
				printf("%d %s %d\n",i,temp->name,temp->depth);
			}
		}
			
}

void initloc()
{
	currentloc = (char*)malloc(sizeof(char)*MAXLEN);
	currentloc = getcwd(currentloc, 100);
}

void goup()
{
	printf("\nWychodze folder wyzej!\n\n");
	chdir("..");
	initloc();
}

void openfolder(char* name)
{
	if (searchforfolder(name) == 1)
	{
		char* temp = (char*)malloc(sizeof(char)*strlen(currentloc));
		char* temp2 = (char*)malloc(sizeof(char)*strlen(currentloc));
		strncpy(temp, currentloc,strlen(currentloc));
		strncpy(temp2, currentloc,strlen(currentloc));
		temp[strlen(temp)] = '/';
		strcpy(currentloc, temp);
		strcat(currentloc, name);
		if(is_regular_file(temp))
		{
			strcpy(currentloc, temp2);
			printf("\nNie ma folderu %s !\n\n", name);
		}
		else
		{
			printf("\nWchodze do folderu %s !\n\n", name);
			chdir(name);
		}	
			
	}
	else
		printf("\nNie ma folderu %s !\n\n", name);
}


void menu()
{
	printf("\n\nYour current location: %s \n\n", currentloc);
	printf("------------MENU(enter number between 0 and 9)------------\n");
	printf("0.GO up\n");
	printf("1.Read current dir\n");
	printf("2.Open folder by name\n");
	printf("3.Search for file by fullname\n");
	printf("4.Search for file by starting part\n");
	printf("5.Search for file by type\n");
	printf("6.Create file\n");
	printf("7.Remove file by name\n");
	printf("8.Read current dir with subdirs\n");
	printf("9.Exit\n");
	int i;
	scanf("%d",&i);
	switch(i)
	{
		case 0:
			goup();
			menu();
			break;
		case 1:
			{
				printdir(currentloc, 0);
				menu();
				break;
			}
		case 2:
			{
				printf("Enter name of folder: ");
				char name[MAXSTRLEN];
				scanf("%s", name);
				openfolder(name);
				printdir(currentloc, 0);
				menu();
				break;
			}
		case 3:
			{
				printf("Enter name of file you are looking for: ");
				char name[MAXSTRLEN];
				scanf("%s", name);
				int check = searchforfile(name);
				if(check != 0)
					printf("Plik %s istnieje\n",name);
				else
					printf("Nie ma pliku %s\n",name);
				menu();
				break;
			}
		case 4:
			{
				printf("Enter partname of file you are looking for: ");
				char name[MAXSTRLEN];
				scanf("%s", name);
				searchforfile_partname(name);
				menu();
				break;
			}
		case 5:
			{
				printf("Enter type of file you are looking for: ");
				char name[MAXSTRLEN];
				scanf("%s", name);
				searchforfile_type(name);
				menu();
				break;
			}
		case 6:
			{
				printf("\nBefore add: \n\n");
				printdir(currentloc, 0);
				printf("Enter name of file you want to create: ");
				char name[MAXSTRLEN];
				scanf("%s", name);
				int check = addfile(name);
				if(check == 0)
					printf("Failed to create file!!!\n");
				else
					printf("Created succesfully!\n");
				printf("\nAfter add: \n\n");
				printdir(currentloc, 0);
				menu();
				break;
			}
		case 7:
			{
				printf("Enter name of file you want to remove: ");
				char name[MAXSTRLEN];
				scanf("%s", name);
				int check = removefile(name);
				if(check == 1)
					printf("Failed to create file!!!\n");
				else
					printf("Removed succesfully!\n");
				menu();
				break;
			}
		case 8:
			{
				init();
				printdirwithsubdirs(currentloc, 0);
				menu();
				break;
			}
		case 9:
			{
				printf("Thanks for using");
				exit(0);
				break;
			}
		default:
			{
				printf("Enter proper value:\n");
				menu();
			}	
	}
}

int main(int argc, char *argv[]) 
{
	initloc();
	printdir(currentloc, 0);
	menu();
	return 0;
}

