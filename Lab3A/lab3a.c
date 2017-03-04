#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>

struct superBlock_t{
    uint16_t* magicnumber;
    uint32_t* inodecount;
    uint32_t* blockcount;
    uint32_t* blocksize;
    int32_t* fragmentsize;
    uint32_t* blockspergroup;
    uint32_t* inodespergroup;
    uint32_t* fragmentspergroup;
    uint32_t* firstdatablock;
};

struct groupDescriptor_t{
    uint32_t numblocks;
    uint16_t numfreeblocks;
    uint16_t numfreeinodes;
    uint16_t numdirs;
    uint32_t freeinodebitmap;
    uint32_t freeblockbitmap;
    uint32_t inodestartblock; 
};

//Initialization of global variables
char* filename;
struct superBlock_t superBlockvar;
struct groupDescriptor_t* groups;
struct freeBitmap_t** freebitmap;
int FILE_SIZE;
int numGroups;
int BLOCK_SIZE;
int INODE_SIZE  = 128;

//returns 0 if tocheck is a power of two, otherwise returns 1
int power_of_two(int tocheck)
{
    if(!(tocheck==0) && !(tocheck & (tocheck-1)))
        return 0;
    else
        return 1;
}

void getSuperBlock(int dskimage)
{
    // uint16_t 16bitpointer;
    // uint32_t 32bitpointer;
    // int32_t s_32bitpointer;
    void* superblock = malloc(264);
    if(pread(dskimage, superblock, 264, 1024) < 0)
    {
        fprintf(stderr, "pread error from reading superblock");
        exit(1);
    }
    superBlockvar.magicnumber = superblock + 56;
    superBlockvar.inodecount = superblock;
    superBlockvar.blockcount = superblock + 4;
    superBlockvar.blocksize = superblock + 24;
    superBlockvar.fragmentsize = superblock + 28;
    superBlockvar.blockspergroup = superblock + 32;
    superBlockvar.inodespergroup = superblock + 40;
    superBlockvar.fragmentspergroup = superblock + 36;
    superBlockvar.firstdatablock = superblock + 20;

    BLOCK_SIZE = 1024 << *(superBlockvar.blocksize);

    //sanity checking
    //is the magic number valid?
    if(*(superBlockvar.magicnumber) != 0xef53)
    {
        fprintf(stderr, "Error - Superblock: invalid magic number: %04x\n", *(superBlockvar.magicnumber));
        exit(1);
    }
    //reasonable block size 
    if(BLOCK_SIZE < 512 || BLOCK_SIZE > 640000 || power_of_two(BLOCK_SIZE))
    {
        fprintf(stderr, "Error - Superblock: invalid block size: %d\n", BLOCK_SIZE);
        exit(1);
    }
    //checking file size constraints
    struct stat filesize;
    fstat(dskimage, &filesize);
    FILE_SIZE = filesize.st_size;
    if((*superBlockvar.blockcount) * BLOCK_SIZE > FILE_SIZE)
    {
        fprintf(stderr, "Error - Superblock: block count * block size: %d, total file size %d\n",(*superBlockvar.blockcount) * BLOCK_SIZE, filesize.st_size);
        exit(1);
    }
    //block count and blocks per group is valid
    if(*(superBlockvar.blockcount)%*(superBlockvar.blockspergroup) != 0)
    {
        fprintf(stderr, "Error - Superblock: %d blocks, %d blocks per group \n", *(superBlockvar.blockcount), *(superBlockvar.blockspergroup));
        exit(1);
    }
    
    //inode count and inodes per group is valid
     if(*(superBlockvar.inodecount)%*(superBlockvar.inodespergroup) != 0)
    {
        fprintf(stderr, "Error - Superblock: %d inodes, %d inodes per group \n", *(superBlockvar.inodecount), *(superBlockvar.inodespergroup));
        exit(1);
    }
}

void printTest()
{
    FILE* dir = fopen("directory.csv", "a");
    fprintf(dir, "Hello\n");
    fclose(dir);
}

void getGroupDescriptor(int dskimage)
{
    FILE *bitmap;
    FILE *inode;
  //  FILE* dir;
    bitmap = fopen("bitmap.csv", "w");
    inode = fopen("inode.csv", "w");
  //  dir = fopen("directory.csv", "a");
    void* groupTable = malloc(32);
    uint32_t* four_byte = malloc(sizeof(uint32_t));
    uint16_t* two_byte = malloc(sizeof(uint16_t));
    uint8_t* one_byte = malloc(sizeof(uint8_t));
    uint8_t* single_block = malloc(BLOCK_SIZE);
    uint8_t single_byte;
    numGroups = *(superBlockvar.blockcount)/(*(superBlockvar.blockspergroup));
    groups = malloc(sizeof(struct groupDescriptor_t)*numGroups);

    
    for(int i = 0; i < numGroups; i++)
    {
        int blockbitmap;
        int inodebitmap;
        //offset depending on the group
        int addrtoread = 2048 + (i*32);
        //get all information necessary for group.csv
        pread(dskimage, two_byte, 2, (addrtoread + 12));
        groups[i].numfreeblocks = *two_byte;
        pread(dskimage, two_byte, 2, (addrtoread + 14));
        groups[i].numfreeinodes = *two_byte;
        pread(dskimage, two_byte, 2, (addrtoread + 16));
        groups[i].numdirs = *two_byte;
        pread(dskimage, four_byte, 4, (addrtoread + 4));
        groups[i].freeinodebitmap = *four_byte;
        inodebitmap = *four_byte;
        pread(dskimage, four_byte, 4, (addrtoread));
        groups[i].freeblockbitmap = *four_byte;
        blockbitmap = *four_byte;
        pread(dskimage, four_byte, 4, (addrtoread + 8));
        groups[i].inodestartblock = *four_byte;
        groups[i].numblocks = *(superBlockvar.blockspergroup);

        //TODO: sanity check for the validity of the inode and freeblockbitmap locations
       
        
        int freeElements = groups[i].numfreeblocks + groups[i].numfreeinodes;
        int freeElementCounter = 0;
        pread(dskimage, single_block, BLOCK_SIZE, BLOCK_SIZE*blockbitmap);
        uint32_t bitmapsize;
        if(i == (numGroups -1))
            bitmapsize =*(superBlockvar.blockspergroup)- (*(superBlockvar.blockcount)) % (*(superBlockvar.blockspergroup));
        else
            bitmapsize = *(superBlockvar.blockspergroup);
       
        //check for free blocks 
        for(uint32_t curr_bit = 0; curr_bit < bitmapsize; curr_bit++)
        {
            single_byte = single_block[curr_bit/8];
            if(!(single_byte & (1 << (curr_bit % 8))))
            {
                fprintf(bitmap, "%x,%d\n", blockbitmap, *(superBlockvar.blockspergroup)*i + curr_bit + 1);
            }
        }

        //checking for free inodes 
        pread(dskimage, single_block, BLOCK_SIZE, BLOCK_SIZE*inodebitmap);
        for(uint32_t curr_bit = 0; curr_bit < *(superBlockvar.inodespergroup); curr_bit++)
        {
            single_byte = single_block[curr_bit/8];
            if(!(single_byte & (1 << (curr_bit % 8))))
            {
                fprintf(bitmap, "%x,%d\n", inodebitmap, *(superBlockvar.inodespergroup)*i + curr_bit + 1);
            }
            else //this means that the node is allcoated and contains content!
            {
                //hard coding that the inode size is 128
                void* inodeTable = malloc(INODE_SIZE);
                uint32_t inode_number = *(superBlockvar.inodespergroup)*i + curr_bit + 1;
                char inode_filetype;
                uint16_t inode_mode;
                uint16_t inode_owner;
                uint16_t inode_group;
                uint16_t inode_link_count;
                uint32_t inode_create_time;
                uint32_t inode_modify_time;
                uint32_t inode_access_time;
                uint32_t inode_file_size;
                uint32_t inode_num_blocks;
                uint32_t inode_block_ptrs[15];
                uint64_t inode_offset = BLOCK_SIZE*groups[i].inodestartblock + INODE_SIZE*curr_bit;
                //read the inode table from disk and assign appropriate variables
                pread(dskimage, inodeTable, INODE_SIZE, inode_offset);
                two_byte = inodeTable;
                if((*(two_byte) & 0x8000)&&(*(two_byte)&0x2000))
                    inode_filetype = 's';
                else if((*(two_byte) & 0x8000))
                    inode_filetype = 'f';
                else if(*(two_byte) & 0x4000)
                {
                    inode_filetype = 'd';
                }
                else
                    inode_filetype = '?';
                inode_mode = *two_byte;
                two_byte = inodeTable + 2;
                inode_owner = *two_byte;
                two_byte = inodeTable + 24;
                inode_group = *two_byte;
                two_byte = inodeTable + 26;
                inode_link_count = *two_byte;
                four_byte = inodeTable + 12;
                inode_create_time = *four_byte;
                four_byte = inodeTable + 8;
                inode_access_time = *four_byte;
                four_byte = inodeTable + 16;
                inode_modify_time = *four_byte;
                four_byte = inodeTable + 4;
                inode_file_size = *four_byte;
                four_byte = inodeTable + 28;
                inode_num_blocks = *four_byte / (2 << BLOCK_SIZE);

                //get the data for the 15 element array
                for(int j = 0; j < 15; j++)
                {
                    four_byte = inodeTable + 40 + j*4;
                    //sanity check for inode block numbers
                    if((*four_byte) * BLOCK_SIZE > FILE_SIZE)
                    {
                        fprintf(stderr, "Error - Inode: Invalid block pointer[%d]: %d\n",j,*four_byte);
                        exit(1);
                    }
                    inode_block_ptrs[j] = *four_byte;
                }

               
                //if it's a directory - it's time to fill out the directory csv
                if(inode_filetype == 'd')
                {
                    //getDirectory(inode_number, inode_num_blocks, inode_block_ptrs, dskimage);
                    FILE *directory;
                    FILE *indirect;
                    int entry_counter = 0;
                    void* dirEntry = malloc(8);
                    uint32_t inode_num;
                    uint16_t entry_length;
                    uint8_t name_length;
                    directory = fopen("directory.csv", "a");
                    indirect = fopen("indirect.csv", "a");
                    for(int k = 0; k < inode_num_blocks; k++)
                    {
                        int starting_address = inode_block_ptrs[k]*BLOCK_SIZE;
                        int limit = starting_address + BLOCK_SIZE;
                        int indirect_counter = 0;
                        if(k == 12) //checking the indirect block
                        {
                            if (inode_block_ptrs[k] ==0)
                                k++; //no double indirect block exists
                            else
                            {  
                                int indirectaddress = inode_block_ptrs[k];
                                int indirect_limit = indirectaddress + BLOCK_SIZE;
                                uint32_t* four_byte_indirect = malloc(sizeof(uint32_t));
                                uint16_t* two_byte_indirect = malloc(sizeof(uint16_t));
                                uint8_t* one_byte_indirect = malloc(sizeof(uint8_t));
                                void* indirect_block = malloc(BLOCK_SIZE);
                                pread(dskimage, indirect_block, BLOCK_SIZE, BLOCK_SIZE*inode_block_ptrs[k]);
                                //check for the limiting number of elements
                                int limit =0;
                                for(int h = 0; h < inode_num_blocks; h++)
                                {
                                    four_byte_indirect = indirect_block + h*4;
                                    if(*(four_byte_indirect) == 0)
                                        limit = h;
                                }
                                for(int j = k; j < limit; j++)
                                {
                                    four_byte_indirect = indirect_block + (j-12)*4;
                                    int starting_address_2 = *four_byte_indirect * BLOCK_SIZE;
                                    int limit_2 = starting_address_2 + BLOCK_SIZE;

                                    //add this block to indirect.csv
                                    fprintf(indirect, "%x,%d,%x\n", inode_block_ptrs[k],indirect_counter,*four_byte_indirect);
                                    indirect_counter++;

                                    while(starting_address_2 < limit_2)
                                    {
                                        pread(dskimage, dirEntry, 8, starting_address_2);
                                        four_byte_indirect = dirEntry;
                                        inode_num = *four_byte_indirect;
                                        two_byte_indirect = dirEntry + 4;
                                        entry_length = *two_byte_indirect;
                                        one_byte_indirect = dirEntry + 6;
                                        name_length = *one_byte_indirect;

                                        if(inode_num == 0)
                                        {
                                            entry_counter++;
                                            starting_address_2 = starting_address_2 + entry_length;
                                            continue;
                                        } 
                                        
                                        //check if entry length is reasonable
                                        if(entry_length < 8 || entry_length > 1024)
                                        {
                                            fprintf(stderr, "Inode - Entry length of %d is not reasonable.\n", entry_length);
                                            exit(1);
                                        }
                                        //check to see that the name length fits inside the entry length 
                                        if(name_length > entry_length)
                                        {
                                            fprintf(stderr, "Name length %d exceeds entry length %d\n", name_length,entry_length);
                                            exit(1);
                                        }

                                        //check to see that inode exists inside the file system
                                        if(inode_num > *(superBlockvar.inodecount))
                                        {
                                            fprintf(stderr, "Inode number %d exceeds inode count %d\n", inode_num, *(superBlockvar.inodecount));
                                            exit(1);
                                        }

                                        fprintf(directory, "%d,%d,%d,%d,%d,", inode_number, entry_counter, entry_length, name_length, inode_num);
                                        char* name_indirect = malloc(name_length);
                                        pread(dskimage, name_indirect, name_length, starting_address_2 + 8);
                                        name_indirect[name_length]  = 0;
                                        fprintf(directory, "\"%s\"\n", name_indirect);
                                        entry_counter++;
                                        starting_address_2 = starting_address_2 + entry_length;

                                    }
                                }
                            // k = inode_num_blocks;
                            }
                        }
                        else if(k==13) //doubly indirect blocks
                        {
                             if (inode_block_ptrs[k] ==0)
                                k++; //no double indirect block exists
                            else
                            {
                                uint32_t* four_byte_indirect = malloc(sizeof(uint32_t));
                                uint16_t* two_byte_indirect = malloc(sizeof(uint16_t));
                                uint8_t* one_byte_indirect = malloc(sizeof(uint8_t));
                                void* indirect_block = malloc(BLOCK_SIZE);
                                int double_limit = 0;
                                pread(dskimage, indirect_block, BLOCK_SIZE, BLOCK_SIZE*inode_block_ptrs[k]);
                                for(int h2 = 0; h2 < 256; h2++)
                                {
                                    four_byte_indirect = indirect_block + h2*4;
                                    if(*(four_byte_indirect) == 0)
                                        double_limit = h2;
                                }
                                for(int g = 0; g < double_limit; g++)
                                {
                                    int* indirectaddress = indirect_block + g*4;
                                    int indirect_limit = *indirectaddress + BLOCK_SIZE;
                                    void* indirect_block = malloc(BLOCK_SIZE);
                                    pread(dskimage, indirect_block, BLOCK_SIZE, BLOCK_SIZE*inode_block_ptrs[k]);
                                    //check for the limiting number of elements
                                    int limit =0;
                                    for(int h = 0; h < inode_num_blocks; h++)
                                    {
                                        four_byte_indirect = indirect_block + h*4;
                                        if(*(four_byte_indirect) == 0)
                                            limit = h;
                                    }
                                    for(int j = k; j < limit; j++)
                                    {
                                        four_byte_indirect = indirect_block + (j-12)*4;
                                        int starting_address_2 = *four_byte_indirect * BLOCK_SIZE;
                                        int limit_2 = starting_address_2 + BLOCK_SIZE;

                                        //add this block to indirect.csv
                                        fprintf(indirect, "%x,%d,%x\n", inode_block_ptrs[k],indirect_counter,*four_byte_indirect);
                                        indirect_counter++;

                                        while(starting_address_2 < limit_2)
                                        {
                                            pread(dskimage, dirEntry, 8, starting_address_2);
                                            four_byte_indirect = dirEntry;
                                            inode_num = *four_byte_indirect;
                                            two_byte_indirect = dirEntry + 4;
                                            entry_length = *two_byte_indirect;
                                            one_byte_indirect = dirEntry + 6;
                                            name_length = *one_byte_indirect;

                                            if(inode_num == 0)
                                            {
                                                entry_counter++;
                                                starting_address_2 = starting_address_2 + entry_length;
                                                continue;
                                            } 
                                            
                                            //check if entry length is reasonable
                                            if(entry_length < 8 || entry_length > 1024)
                                            {
                                                fprintf(stderr, "Inode - Entry length of %d is not reasonable.\n", entry_length);
                                                exit(1);
                                            }
                                            //check to see that the name length fits inside the entry length 
                                            if(name_length > entry_length)
                                            {
                                                fprintf(stderr, "Name length %d exceeds entry length %d\n", name_length,entry_length);
                                                exit(1);
                                            }

                                            //check to see that inode exists inside the file system
                                            if(inode_num > *(superBlockvar.inodecount))
                                            {
                                                fprintf(stderr, "Inode number %d exceeds inode count %d\n", inode_num, *(superBlockvar.inodecount));
                                                exit(1);
                                            }

                                            fprintf(directory, "%d,%d,%d,%d,%d,", inode_number, entry_counter, entry_length, name_length, inode_num);
                                            char* name_indirect = malloc(name_length);
                                            pread(dskimage, name_indirect, name_length, starting_address_2 + 8);
                                            name_indirect[name_length]  = 0;
                                            fprintf(directory, "\"%s\"\n", name_indirect);
                                            entry_counter++;
                                            starting_address_2 = starting_address_2 + entry_length;

                                        }
                                    }
                                }
                            }
                        }
                        else if(k==14) //triple indirect blocks
                        {
                             if (inode_block_ptrs[k] ==0)
                                k++;//no triple indirect block exists
                            else
                            {
                                uint32_t* four_byte_indirect = malloc(sizeof(uint32_t));
                                uint16_t* two_byte_indirect = malloc(sizeof(uint16_t));
                                uint8_t* one_byte_indirect = malloc(sizeof(uint8_t));
                                void* indirect_block = malloc(BLOCK_SIZE);
                                int triple_limit = 0;
                                pread(dskimage, indirect_block, BLOCK_SIZE, BLOCK_SIZE*inode_block_ptrs[k]);
                                for(int h3 = 0; h3 < 256; h3++)
                                {
                                    four_byte_indirect = indirect_block + h3*4;
                                    if(*(four_byte_indirect) == 0)
                                    triple_limit = h3;
                                }
                                for(int f = 0; f < triple_limit; f++)
                                {
                                    int double_limit = 0;
                                    pread(dskimage, indirect_block, BLOCK_SIZE, BLOCK_SIZE*inode_block_ptrs[k]);
                                    for(int h2 = 0; h2 < 256; h2++)
                                    {
                                        four_byte_indirect = indirect_block + h2*4;
                                        if(*(four_byte_indirect) == 0)
                                            double_limit = h2;
                                    }
                                    for(int g = 0; g < double_limit; g++)
                                    {
                                        int* indirectaddress = indirect_block + g*4;
                                        int indirect_limit = *indirectaddress + BLOCK_SIZE;
                                        void* indirect_block = malloc(BLOCK_SIZE);
                                        pread(dskimage, indirect_block, BLOCK_SIZE, BLOCK_SIZE*inode_block_ptrs[k]);
                                        //check for the limiting number of elements
                                        int limit =0;
                                        for(int h = 0; h < inode_num_blocks; h++)
                                        {
                                            four_byte_indirect = indirect_block + h*4;
                                            if(*(four_byte_indirect) == 0)
                                                limit = h;
                                        }
                                        for(int j = k; j < limit; j++)
                                        {
                                            four_byte_indirect = indirect_block + (j-12)*4;
                                            int starting_address_2 = *four_byte_indirect * BLOCK_SIZE;
                                            int limit_2 = starting_address_2 + BLOCK_SIZE;

                                            //add this block to indirect.csv
                                            fprintf(indirect, "%x,%d,%x\n", inode_block_ptrs[k],indirect_counter,*four_byte_indirect);
                                            indirect_counter++;

                                            while(starting_address_2 < limit_2)
                                            {
                                                pread(dskimage, dirEntry, 8, starting_address_2);
                                                four_byte_indirect = dirEntry;
                                                inode_num = *four_byte_indirect;
                                                two_byte_indirect = dirEntry + 4;
                                                entry_length = *two_byte_indirect;
                                                one_byte_indirect = dirEntry + 6;
                                                name_length = *one_byte_indirect;

                                                if(inode_num == 0)
                                                {
                                                    entry_counter++;
                                                    starting_address_2 = starting_address_2 + entry_length;
                                                    continue;
                                                } 
                                                
                                                //check if entry length is reasonable
                                                if(entry_length < 8 || entry_length > 1024)
                                                {
                                                    fprintf(stderr, "Inode - Entry length of %d is not reasonable.\n", entry_length);
                                                    exit(1);
                                                }
                                                //check to see that the name length fits inside the entry length 
                                                if(name_length > entry_length)
                                                {
                                                    fprintf(stderr, "Name length %d exceeds entry length %d\n", name_length,entry_length);
                                                    exit(1);
                                                }

                                                //check to see that inode exists inside the file system
                                                if(inode_num > *(superBlockvar.inodecount))
                                                {
                                                    fprintf(stderr, "Inode number %d exceeds inode count %d\n", inode_num, *(superBlockvar.inodecount));
                                                    exit(1);
                                                }

                                                fprintf(directory, "%d,%d,%d,%d,%d,", inode_number, entry_counter, entry_length, name_length, inode_num);
                                                char* name_indirect = malloc(name_length);
                                                pread(dskimage, name_indirect, name_length, starting_address_2 + 8);
                                                name_indirect[name_length]  = 0;
                                                fprintf(directory, "\"%s\"\n", name_indirect);
                                                entry_counter++;
                                                starting_address_2 = starting_address_2 + entry_length;

                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else{
                            if(k ==15)
                            {
                                k = inode_num_blocks;
                                continue;
                            }
                            else{
                                while(starting_address < limit)
                                {
                                    //get inode number
                                    pread(dskimage, dirEntry, 8, starting_address);
                                    four_byte = dirEntry;
                                    inode_num = *four_byte;
                                    two_byte = dirEntry + 4;
                                    entry_length = *two_byte;
                                    one_byte = dirEntry + 6;
                                    name_length = *one_byte;

                                    if(inode_num == 0)
                                    {
                                        entry_counter++;
                                        starting_address = starting_address + entry_length;
                                        continue;
                                    }
                                    //print information except name
                                    fprintf(directory, "%d,%d,%d,%d,%d,", inode_number, entry_counter, entry_length, name_length, inode_num);

                                    char* name = malloc(name_length);
                                    pread(dskimage, name, name_length, starting_address + 8);
                                    name[name_length] = 0;

                                    fprintf(directory, "\"%s\"\n", name);

                                    entry_counter++;
                                    starting_address = starting_address + entry_length;

                                }
                            }
                        }
                    }
                    fclose(directory);  
                    fclose(indirect);  
                }

                //print out everything except the array
                fprintf(inode, "%d,%c,%o,%d,%d,%d,%x,%x,%x,%d,%d,", inode_number, inode_filetype, inode_mode, inode_owner, inode_group, inode_link_count, inode_create_time, inode_modify_time, inode_access_time, inode_file_size, inode_num_blocks);
                //print out the array and add a newline for the next entry
                for(int j = 0; j < 15; j++)
                {
                    if(j == 14) //if its the last entry then that means we can use a newline
                        fprintf(inode, "%x\n", inode_block_ptrs[j]);
                    else
                        fprintf(inode, "%x,", inode_block_ptrs[j]);
                }
            }
        }
     
    }
    //Free bitmap operations and close files
    fclose(bitmap);
    fclose(inode);
}

void printCSVfiles()
{
    //TODO: check for failure in file opens 
    
    FILE* fp = fopen("super.csv", "w");
    int32_t fragsize = *(superBlockvar.fragmentsize);
    if(fragsize < 0)
        fragsize = 1024 >> -fragsize;
    else
        fragsize = 1024 << fragsize;
    fprintf(fp, "%04x,%d,%d,%d,%d,%d,%d,%d,%d\n", *(superBlockvar.magicnumber), *(superBlockvar.inodecount), *(superBlockvar.blockcount), 1024 << *(superBlockvar.blocksize), fragsize, *(superBlockvar.blockspergroup), *(superBlockvar.inodespergroup), *(superBlockvar.fragmentspergroup), *(superBlockvar.firstdatablock));
    fclose(fp);

    fp = fopen("group.csv", "w");
    for(int i = 0; i < numGroups; i++)
    {
        fprintf(fp, "%d,%d,%d,%d,%x,%x,%x\n", groups[i].numblocks, groups[i].numfreeblocks, groups[i].numfreeinodes, groups[i].numdirs, groups[i].freeinodebitmap, groups[i].freeblockbitmap, groups[i].inodestartblock);
    }
    fclose(fp);
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Please use only one arguement with lab3a.");
        exit(1);
    }
    
    int index = optind;
    filename = argv[optind];
    
    int dskimage = open(filename, O_RDONLY);
    if(dskimage < 0)
    {
        fprintf(stderr, "Error opening file.");
        exit(1);
    }

    getSuperBlock(dskimage);
    getGroupDescriptor(dskimage);
    printCSVfiles();
    

}