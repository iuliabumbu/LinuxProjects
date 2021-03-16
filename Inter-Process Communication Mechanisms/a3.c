#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <math.h>
#define FIFO_RESP "RESP_PIPE_43465"
#define FIFO_REQ "REQ_PIPE_43465"

int checkValidFile(char *magic, unsigned short version, unsigned int noSect, unsigned int sectType){

	if(strcmp(magic, "0Uvf") != 0)
	{
		return 1;
	}

	if(version< 20 || version >87)
	{
		return 1;
	}

	if(noSect< 10 || noSect >12)
	{
		return 1;
	}

	if(sectType != 38 && sectType != 17)
	{
		return 1;
	}

	return 0;
}

int main()
{
	int fdResp = -1;
	int fdReq = -1;

	unlink(FIFO_RESP);
	shm_unlink("/IjCqBA9");

	if (mkfifo(FIFO_RESP, 0644) != 0)
	{
		printf("ERROR\ncannot create the response pipe\n");
		return 1;
	}

	fdReq = open(FIFO_REQ, O_RDONLY);
	if (fdReq == -1)
	{
		printf("ERROR\ncannot open the request pipe\n");
		return 1;
	}

	fdResp = open(FIFO_RESP, O_WRONLY);
	if (fdResp == -1)
	{
		printf("ERROR\ncannot open the response pipe\n");
		return 1;
	}

	if (write(fdResp, "\x07", 1) == 1 && write(fdResp, "CONNECT", 7) == 7)
	{
		printf("SUCCESS\n");
	}

	int req_lung = 0;
	int file_lung = 0;
	int file_size = 0;
	int shmFd = 0;
	int fileFd;
	unsigned int nr_ping = 0;
	unsigned int mem_size = 0;
	unsigned char *sharedVal = NULL;
	unsigned char *fileArea = NULL;

	while (1)
	{
		read(fdReq, &req_lung, 1);
		char req_name[req_lung];
		read(fdReq, req_name, req_lung);
		req_name[req_lung] = '\0';

		if (strcmp(req_name, "EXIT") == 0)
		{
			final: close(fdResp);
			close(fdReq);
			unlink(FIFO_RESP);
			munmap(sharedVal, mem_size);
			sharedVal = NULL;
			munmap(fileArea, file_size);
			fileArea = NULL;
			shm_unlink("/IjCqBA9");
			close(fileFd); 
			return 0;
		}

		if (strcmp(req_name, "PING") == 0)
		{
			write(fdResp, "\x04", 1);
			write(fdResp, "PING", 4);
			write(fdResp, "\x04", 1);
			write(fdResp, "PONG", 4);
			nr_ping = 43465;
			write(fdResp, &nr_ping, sizeof(unsigned int));
		}

		if (strcmp(req_name, "CREATE_SHM") == 0)
		{

			shmFd = shm_open("/IjCqBA9", O_CREAT | O_RDWR, 0664);
			read(fdReq, &mem_size, sizeof(unsigned int));

			if (shmFd < 0)
			{
				printf("Could not aquire shm");
				write(fdResp, "\x0A", 1);
				write(fdResp, "CREATE_SHM", 10);
				write(fdResp, "\x05", 1);
				write(fdResp, "ERROR", 5);
				goto final;
			}

			write(fdResp, "\x0A", 1);
			write(fdResp, "CREATE_SHM", 10);
			write(fdResp, "\x07", 1);
			write(fdResp, "SUCCESS", 7);

			ftruncate(shmFd, mem_size);

			sharedVal = (unsigned char *) mmap(0, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);

			if (sharedVal == (void*) - 1)
			{
				write(fdResp, "\x0A", 1);
				write(fdResp, "CREATE_SHM", 10);
				write(fdResp, "\x05", 1);
				write(fdResp, "ERROR", 5);
				goto final;
			}

		}

		if (strcmp(req_name, "WRITE_TO_SHM") == 0)
		{
			unsigned int offset = 0;
			unsigned int val = 0;
			read(fdReq, &offset, sizeof(unsigned int));
			read(fdReq, &val, sizeof(unsigned int));

			if (0 <= offset && offset< mem_size && (offset + sizeof(val)) < mem_size)
			{
				unsigned int *value = (unsigned int *)(&sharedVal[offset]);
				*value = val;

				write(fdResp, "\x0C", 1);
				write(fdResp, "WRITE_TO_SHM", 12);
				write(fdResp, "\x07", 1);
				write(fdResp, "SUCCESS", 7);
			}
			else
			{
				write(fdResp, "\x0C", 1);
				write(fdResp, "WRITE_TO_SHM", 12);
				write(fdResp, "\x05", 1);
				write(fdResp, "ERROR", 5);
				goto final;
			}
		}

		if (strcmp(req_name, "MAP_FILE") == 0)
		{

			read(fdReq, &file_lung, 1);
			char file_name[file_lung];
			read(fdReq, file_name, file_lung);
			file_name[file_lung] = '\0';


			fileFd = open(file_name, O_RDONLY);

			if (fileFd == -1)
			{
				printf("Could not open input file\n");
				write(fdResp, "\x08", 1);
				write(fdResp, "MAP_FILE", 8);
				write(fdResp, "\x05", 1);
				write(fdResp, "ERROR", 5);
				goto final;
			}

			file_size = lseek(fileFd, 0, SEEK_END);
			lseek(fileFd, 0, SEEK_SET);

			fileArea = (unsigned char *) mmap(0, file_size, PROT_READ, MAP_PRIVATE, fileFd, 0);

			if (fileArea == (void*) - 1)
			{
				write(fdResp, "\x08", 1);
				write(fdResp, "MAP_FILE", 8);
				write(fdResp, "\x05", 1);
				write(fdResp, "ERROR", 5);
				goto final;
			}
			write(fdResp, "\x08", 1);
			write(fdResp, "MAP_FILE", 8);
			write(fdResp, "\x07", 1);
			write(fdResp, "SUCCESS", 7);
		}

		if (strcmp(req_name, "READ_FROM_FILE_OFFSET") == 0)
		{
			unsigned int offset = 0;
			unsigned int noBytes = 0;
			read(fdReq, &offset, sizeof(unsigned int));
			read(fdReq, &noBytes, sizeof(unsigned int));

			unsigned short headerSize = *(unsigned short *)(fileArea + file_size - 6);
			char magic[4];
			magic[0] = *(fileArea + file_size - 4);
			magic[1] = *(fileArea + file_size - 3);
			magic[2] = *(fileArea + file_size - 2);
			magic[3] = *(fileArea + file_size - 1);
			magic[4] = '\0';
			unsigned short version = *(unsigned short *)(fileArea + file_size -headerSize);
			unsigned int noSections = *(fileArea + file_size - headerSize + 2);
			unsigned int sectType;
			for(int section = 1; section <noSections; section ++)
			{
				sectType= *(fileArea + file_size - headerSize + 3 + (section - 1) * 26 + 17);

				if(checkValidFile(magic, version, noSections, sectType) != 0)
				{
					write(fdResp, "\x15", 1);
				    write(fdResp, "READ_FROM_FILE_OFFSET", 21);
				    write(fdResp, "\x05", 1);
				    write(fdResp, "ERROR", 5);
					goto final;

				}
			}

			unsigned char *data = (unsigned char *) malloc(noBytes);

			if ((offset + noBytes) < file_size)
			{

				for (int i = 0; i < noBytes; i++)
				{
					data[i] = fileArea[offset + i];
				}

				for (int i = 0; i < noBytes; i++)
				{
					sharedVal[i] = data[i];
				}

				write(fdResp, "\x15", 1);
				write(fdResp, "READ_FROM_FILE_OFFSET", 21);
				write(fdResp, "\x07", 1);
				write(fdResp, "SUCCESS", 7);
			}
			else
			{
				write(fdResp, "\x15", 1);
				write(fdResp, "READ_FROM_FILE_OFFSET", 21);
				write(fdResp, "\x05", 1);
				write(fdResp, "ERROR", 5);
			}

			free(data);
		}

		if (strcmp(req_name, "READ_FROM_FILE_SECTION") == 0)
		{    
				unsigned int section = 0;
				unsigned int offset = 0;
				unsigned int noBytes = 0;
				read(fdReq, &section, sizeof(unsigned int));
				read(fdReq, &offset, sizeof(unsigned int));
				read(fdReq, &noBytes, sizeof(unsigned int));

         		unsigned short headerSize = *(unsigned short *)(fileArea + file_size - 6);
				char magic[4];
				magic[0] = *(fileArea + file_size - 4);
				magic[1] = *(fileArea + file_size - 3);
				magic[2] = *(fileArea + file_size - 2);
				magic[3] = *(fileArea + file_size - 1);
				magic[4] = '\0';
			    unsigned short version = *(unsigned short *)(fileArea + file_size -headerSize);
				unsigned int noSections = *(fileArea + file_size - headerSize + 2);
				unsigned int sectType = *(fileArea + file_size - headerSize + 3 + (section - 1) * 26 + 17);

				char *data = (char*)malloc(noBytes);
			   
				if(checkValidFile(magic, version, noSections, sectType) == 0 && section >= 1 && section <= noSections)
				{
                    unsigned int val = *(unsigned int *)(fileArea + file_size - headerSize + 3 + (section - 1) * 26 + 18);

					for(int i = 0; i < noBytes; i++)
					{   
						data[i] = fileArea[val + offset + i];
					}

					for (int i = 0; i < noBytes; i++)
				    {
					    sharedVal[i] = data[i];
				    }


					write(fdResp, "\x16", 1);
				    write(fdResp, "READ_FROM_FILE_SECTION", 22);
					write(fdResp, "\x07", 1);
				    write(fdResp, "SUCCESS", 7);
				}
				else
				{ 		
					write(fdResp, "\x16", 1);
				    write(fdResp, "READ_FROM_FILE_SECTION", 22);
					write(fdResp, "\x05", 1);
				    write(fdResp, "ERROR", 5);
				}

				free(data); 
		}

		if (strcmp(req_name, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
		{
			unsigned int logicalOffset = 0;
			unsigned int noBytes = 0;
			read(fdReq, &logicalOffset, sizeof(unsigned int));
			read(fdReq, &noBytes, sizeof(unsigned int));
		
			unsigned short headerSize = *(unsigned short *)(fileArea + file_size - 6);
			char magic[4];
			magic[0] = *(fileArea + file_size - 4);
			magic[1] = *(fileArea + file_size - 3);
			magic[2] = *(fileArea + file_size - 2);
			magic[3] = *(fileArea + file_size - 1);
			magic[4] = '\0';
			unsigned short version = *(unsigned short *)(fileArea + file_size -headerSize);
			unsigned int noSections = *(fileArea + file_size - headerSize + 2);
			unsigned int sectType;
			for(int section = 1; section <noSections; section ++)
			{
				sectType= *(fileArea + file_size - headerSize + 3 + (section - 1) * 26 + 17);

				if(checkValidFile(magic, version, noSections, sectType) != 0)
				{
					write(fdResp, "\x1E", 1);
				    write(fdResp, "READ_FROM_LOGICAL_SPACE_OFFSET", 30);
				    write(fdResp, "\x05", 1);
				    write(fdResp, "ERROR", 5);
					goto final;

				}
			}
			char *data = (char*)malloc(noBytes);

			unsigned int currentOffset = 0;
			unsigned int sum = 0;
			unsigned int mySect = 0;

				if(logicalOffset + noBytes<file_size)
				{
					for(int section = 1; section <= noSections; section++)
					{   
						unsigned int sectSize = *(unsigned int *)(fileArea + file_size - headerSize + 3 + (section - 1) * 26 + 22);
						sum += sectSize;

						while(currentOffset < sum){
							currentOffset += 1024;
						}
						sum = currentOffset;

						if(currentOffset >= logicalOffset){
							mySect = section;
						
							unsigned int lastCurrent = currentOffset - sectSize - (currentOffset - sectSize) % 1024;
						
							unsigned int logOffset = logicalOffset - lastCurrent;

							unsigned int offset = *(unsigned int *)(fileArea + file_size - headerSize + 3 + (mySect- 1) * 26 + 18);

					        for(int i = 0; i < noBytes; i++)
					        {   
						        data[i] = fileArea[offset + logOffset + i];
					        }
							break;
						}

					}

					for (int i = 0; i < noBytes; i++)
				    {
					    sharedVal[i] = data[i];
				    }


					write(fdResp, "\x1E", 1);
				    write(fdResp, "READ_FROM_LOGICAL_SPACE_OFFSET", 30);
					write(fdResp, "\x07", 1);
				    write(fdResp, "SUCCESS", 7);
				}
				else
				{ 		
					write(fdResp, "\x1E", 1);
				    write(fdResp, "READ_FROM_LOGICAL_SPACE_OFFSET", 30);
					write(fdResp, "\x05", 1);
				    write(fdResp, "ERROR", 5);
				}

				free(data); 
		}
	}

	return 0;

}