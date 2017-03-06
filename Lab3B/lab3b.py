import csv
with open('super.csv','rb') as superfile:
    reader = csv.reader(superfile, delimiter=',')
    super_csv =[]
    number = 0
    for row in reader:
        for num in range(0,9):
            super_csv.append(row[num])

TOTAL_INODES = super_csv[1]
TOTAL_BLOCKS = super_csv[2]
BLOCK_SIZE = int(super_csv[3])
BLOCKS_PER_GROUP = super_csv[5]
INODES_PER_GROUP = super_csv[6]
FIRST_DATA_BLOCK = super_csv[8]

group_bitmap_blocks = []
group_bitmap_inodes =[]
free_block_bitmap_entries = []
free_inode_bitmap_entries = []

txtfile=open('./lab3b_check.txt', 'w+')
                  
#Finding duplicately allocated blocks

inode_csv = []
group_csv = []
bitmap_csv = []
directory_csv = []
indirect_csv = []

with open('group.csv','rb') as groupcsv:
    reader = csv.reader(groupcsv, delimiter=',')
    for row in reader:
        group_csv.append(row)

with open('bitmap.csv','rb') as bitmapcsv:
    reader = csv.reader(bitmapcsv, delimiter=',')
    for row in reader:
        bitmap_csv.append(row)

with open('directory.csv','rb') as directorycsv:
    reader = csv.reader(directorycsv, delimiter=',')
    for row in reader:
        directory_csv.append(row)

with open('indirect.csv','rb') as indirectcsv:
    reader = csv.reader(indirectcsv, delimiter=',')
    for row in reader:
        indirect_csv.append(row)

with open('inode.csv','rb') as inode_dup:
    inode_data = {}
    reader = csv.reader(inode_dup, delimiter=',')
    for row in reader:
        inode_csv.append(row)
        for num in range(11,26):
            if row[num] != '0':
                inode_data.setdefault(row[num],[]).append(row[0])

#find unallocated blocks 
group_bitmap_blocks = []
free_block_bitmap_entries = []
free_inode_bitmap_entries = []
for row in group_csv:
    group_bitmap_blocks.append(row[5])

for row in bitmap_csv:
    if row[0] in group_bitmap_blocks:
        free_block_bitmap_entries.append(row[1])
    else:
        free_inode_bitmap_entries.append(row[1]);

counter = 0

for row in inode_csv:
    for num in range(11,26):
        if str(int(row[num], 16)) in free_block_bitmap_entries:
            txtfile.write('UNALLOCATED BLOCK < ' + str(int(row[num],16)) + ' > REFERENCED BY INODE < ' + str(row[0]) + ' > ENTRY < ' + str(num -11) + ' >\n')
            counter = counter + 1
    if counter == 1:
        break
    #REMOVE THIS BEFORE SUBMISSION



#find multiply referenced inodes
for key in inode_data:
    test_list = []
    test_list = inode_data.get(key)
    if len(test_list) > 1:
        txtfile.write('MULTIPLY REFERENCED BLOCK < %d > ' % int(key,16))
        for inode in test_list:
            txtfile.write('BY INODE < ' + inode + ' >')
            for row in inode_csv:
                if row[0] == inode:
                    txtfile.write(' ENTRY < %d > ' % (int(row.index(key))-11))
        txtfile.write('\n')

#find unallocated inodes 
in_use_inodes = []
for row in inode_csv:
    in_use_inodes.append(row[0])

for row in directory_csv:
    if row[4] not in in_use_inodes:
        txtfile.write('UNALLOCATED INODE < ' + str(row[4]) + ' > REFERENCED BY DIRECTORY < ' + str(row[0]) + ' > ENTRY < ' + str(row[1]) + ' >')

#find missing inodes 




                    
