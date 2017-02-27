#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

struct freeBitmap_t{
    uint32_t mapnum;
    uint32_t freeblocknum;
};

//Initialization of global variables
char* filename;
struct superBlock_t superBlockvar;
struct groupDescriptor_t* groups;
struct freeBitmap_t** freebitmap;
int filesize;
int numGroups;

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

void getGroupDescriptor(int dskimage)
{
    void* groupTable = malloc(32);
    uint32_t* four_byte = malloc(sizeof(uint32_t));
    uint16_t* two_byte = malloc(sizeof(uint16_t));
    numGroups = *(superBlockvar.blockcount)/(*(superBlockvar.blockspergroup));
    groups = malloc(sizeof(struct groupDescriptor_t)*numGroups);
    for(int i = 0; i < numGroups; i++)
    {
        int blockbitmap;
        int inodebitmap;
        int addrtoread = 2048 + (i*32);
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
        freebitmap[i] = malloc(sizeof(struct freeBitmap_t)*freeElements);
        // for(int j = 0; j < freeElements; j++)
        // {
        //     freebitmap[i][j].mapnum = blockbitmap;
        // }
    }
    //Free bitmap operations

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
    fprintf(stdout, "Magic number: %04x\n", *(superBlockvar.magicnumber));

}