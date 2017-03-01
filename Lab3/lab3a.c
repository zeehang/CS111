#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
int filesize;
int numGroups;
int BLOCK_SIZE;
int INODE_SIZE  = 128;

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

    // //sanity checking
    // if(*(superBlockvar.magicnumber) != 0xef53)
    // {
    //     fprintf(stderr, "Error - Superblock: invalid magic number: %04x\n", *(superBlockvar.magicnumber));
    //     exit(1);
    // }
    // //reasonable block size 
    // //TODO: function to check for a power of two
    // if(*(superBlockvar.blocksize) < 512 || *(superBlockvar.blocksize) < 640000 || *(superBlockvar.blocksize)%2 != 0)
    // {
    //     fprintf(stderr, "Error - Superblock: invalid block size: %d\n", *(superBlockvar.blocksize));
    //     exit(1);
    // }
    // //TODO: check file size constraints for sanity!
    // if(*(superBlockvar.blockcount)%*(superBlockvar.blockspergroup) != 0)
    // {
    //     fprintf(stderr, "Error - Superblock: %d blocks, %d blocks per group \n", *(superBlockvar.blockcount), *(superBlockvar.blockspergroup));
    //     exit(1);
    // }
    //  if(*(superBlockvar.inodecount)%*(superBlockvar.inodespergroup) != 0)
    // {
    //     fprintf(stderr, "Error - Superblock: %d inodes, %d inodes per group \n", *(superBlockvar.inodecount), *(superBlockvar.inodespergroup));
    //     exit(1);
    // }
}

void getDirectory(uint32_t parent_inode, uint32_t numblocks, uint32_t blockarr[], int dskimage)
{
    FILE* directory;
   // fprintf(stderr, "%d,%d\n", parent_inode, numblocks);
    directory = fopen("directory.csv", "w");
    uint32_t* four_byte = malloc(sizeof(uint32_t));
    uint16_t* two_byte = malloc(sizeof(uint16_t));
    uint8_t* one_byte = malloc(sizeof(uint8_t));
    int entry_counter = 0;
    uint32_t inode_num;
    uint16_t entry_length;
    uint8_t name_length;
    char namechar;
    void* dirEntry = malloc(8);

    for(int i = 0; i < numblocks; i++)
    {
        int starting_address = blockarr[i]*BLOCK_SIZE;
        int limit = starting_address + BLOCK_SIZE;
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
           // fprintf(directory, "%d,%d,%d,%d,%d,", parent_inode, entry_counter, entry_length, name_length, inode_num);
            fprintf(directory, "%d,%d,%d,%d,%d,", parent_inode, entry_counter, entry_length, name_length, inode_num);
            fprintf(stderr, "%d,%d,%d,%d,%d,", parent_inode, entry_counter, entry_length, name_length, inode_num);
            char* name = malloc(name_length);
            pread(dskimage, name, name_length, starting_address + 8);
            name[name_length] = 0;

          //  fprintf(directory, "%s\n", name);
            fprintf(directory, "\"%s\"\n", name);
            fprintf(stderr, "\"%s\"\n", name);
            entry_counter++;
            starting_address = starting_address + entry_length;
        }
    }

    //PREVIOUS IMPLEMENTATION
    // int end_of_block = 0;
    // for(int i = 0; i < numblocks; i++)
    // {
    //     fprintf(stderr, "Entering for loop for parent inode: %d:\n", parent_inode);
    //     uint32_t starting_address = (blockarr[i])*BLOCK_SIZE;
    //     uint32_t limit_address = starting_address + BLOCK_SIZE;
    //     fprintf(stderr, "Starting address %d:\n", starting_address);
    //     fprintf(stderr, "Ending address %d:\n", limit_address);
    //     while(starting_address < limit_address)
    //     {
    //         // pread(dskimage, dirEntry, 9, block)
    //         pread(dskimage, dirEntry, 8, starting_address);
    //         four_byte = dirEntry;
    //         inode_num = *four_byte;
    //         two_byte = dirEntry + 4;
    //         entry_length = *two_byte;
    //         one_byte = dirEntry + 6;
    //         name_length = *one_byte;
    //         char* name = malloc(name_length);
    //         pread(dskimage, &name, name_length, starting_address + 8);
    //         fprintf(stderr, "%d,%d,%d,%d,%d,%s\n", parent_inode, entry_counter, entry_length, name_length, inode_num, name);
    //         if(fprintf(directory, "%d,%d,%d,%d,%d,%s\n", parent_inode, entry_counter, entry_length, name_length, inode_num, name) <0)
    //             fprintf(stderr, "printing failed\n");
    //         entry_counter++;
    //         starting_address = starting_address + entry_length;
    //     }

    
    // }
    // fclose(directory);
}

void getGroupDescriptor(int dskimage)
{
    FILE *bitmap;
    FILE *inode;
    bitmap = fopen("bitmap.csv", "w");
    inode = fopen("inode.csv", "w");
    void* groupTable = malloc(32);
    uint32_t* four_byte = malloc(sizeof(uint32_t));
    uint16_t* two_byte = malloc(sizeof(uint16_t));
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
                    inode_block_ptrs[j] = *four_byte;
                }
   
                //call the directory function!
                if(inode_filetype == 'd')
                {
                    getDirectory(inode_number, inode_num_blocks, inode_block_ptrs, dskimage);
                }

                //print out everything except the array
                fprintf(inode, "%d,%c,%o,%d,%d,%d,%x,%x,%x,%d,%d,", inode_number, inode_filetype, inode_mode, inode_owner, inode_group, inode_link_count, inode_create_time, inode_modify_time, inode_access_time, inode_file_size, inode_num_blocks);
                //print out the array and add a newline for the next entry
                for(int j = 0; j < 15; j++)
                {
                    if(j == 14)
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