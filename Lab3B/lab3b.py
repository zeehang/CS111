import csv
from collections import defaultdict
with open('super.csv','rb') as superfile:
    reader = csv.reader(superfile, delimiter=',')
    super_csv =[]
    number = 0
    for row in reader:
        for num in range(0,9):
            super_csv.append(row[num])

TOTAL_INODES = int(super_csv[1])
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
free_block_bitmap_numbers = []
free_inode_bitmap_numbers = []
for row in group_csv:
    group_bitmap_blocks.append(row[5])

for row in bitmap_csv:
    if row[0] in group_bitmap_blocks:
        free_block_bitmap_entries.append(row[1])
        if row[0] not in free_block_bitmap_numbers:
            free_block_bitmap_numbers.append(row[0])
    else:
        free_inode_bitmap_entries.append(row[1])
        if row[0] not in free_inode_bitmap_numbers:
            free_inode_bitmap_numbers.append(row[0])

counter = 0

for row in inode_csv:
    for num in range(11,26):
        if str(int(row[num], 16)) in free_block_bitmap_entries:
            txtfile.write('UNALLOCATED BLOCK < ' + str(int(row[num],16)) + ' > REFERENCED BY INODE < ' + str(row[0]) + ' > ENTRY < ' + str(num -11) + ' >\n')

#find multiply referenced inodes
for key in inode_data:
    test_list = []
    test_list = inode_data.get(key)
    if len(test_list) > 1:
        txtfile.write('MULTIPLY REFERENCED BLOCK < %d > BY' % int(key,16))
        for inode in test_list:
            txtfile.write(' INODE < ' + inode + ' >')
            for row in inode_csv:
                if row[0] == inode:
                    txtfile.write(' ENTRY < %d >' % (int(row.index(key))-11))
        txtfile.write('\n')

#find unallocated inodes 
in_use_inodes = []
for row in inode_csv:
    in_use_inodes.append(row[0])

for row in directory_csv:
    if row[4] not in in_use_inodes:
        txtfile.write('UNALLOCATED INODE < ' + row[4] + ' > REFERENCED BY DIRECTORY < ' + row[0] + ' > ENTRY < ' + row[1] + ' >\n')

#find missing inodes
inode_database = {}
for number in range(1, TOTAL_INODES):
    inode_database[number] = 0
for entry in free_inode_bitmap_entries:
    inode_database[int(entry)] = 1
for entry in inode_csv:
    if int(entry[0]) > 11:
        test_bit = 0
        for num in range(11, 26):
            test_bit = test_bit | int(entry[num], 16)
        if test_bit == 0:
            inode_database[int(entry[0])] = 0
        else:
            inode_database[int(entry[0])] = 1

#print inode_database[20]

for number in range(12, TOTAL_INODES):
    if inode_database[number] == 0:
        #print str(number)
        #figure out what the list number means
        free_list_index = number/int(INODES_PER_GROUP)
        txtfile.write('MISSING INODE < ' + str(number) + ' > SHOULD BE IN FREE LIST < ' + free_inode_bitmap_numbers[free_list_index]+  ' >\n')

#incorrect link count
inode_link_listing = {}
actual_link_count = defaultdict(int)
for entry in inode_csv:
    inode_link_listing[int(entry[0])] = int(entry[5])

for entry in directory_csv:
    actual_link_count[int(entry[4])] = actual_link_count[int(entry[4])] + 1

for key, value in inode_link_listing.items():
    if actual_link_count[key] != value:
        txtfile.write('LINKCOUNT < ' + str(key) + ' > IS < ' + str(value) + ' > SHOULD BE < ' + str(actual_link_count[key]) + ' >\n')

#find incorrect directory entry
for entry in directory_csv:
    if entry[5] == '.' and entry[3] == '1':
        if int(entry[0]) != int(entry[4]):
            txtfile.write('INCORRECT ENTRY IN < ' + entry[0] + ' > NAME < . > LINK TO < ' + entry[4] + ' > SHOULD BE < ' + entry[0] + ' >\n')
    if entry[5] == '..':
        foundflag = False
        for dubs in directory_csv:
            if dubs[0] == entry[4] and dubs[5] != '.' and dubs[5] != '..':
                foundflag = True
        if foundflag == False:
            for replace in directory_csv:
                if replace[4] == entry[0] and replace[5] != '.' and replace[5] != '..':
                    txtfile.write('INCORRECT ENTRY IN < ' + entry[0] + ' > NAME < .. > LINK TO < ' + entry[4] + ' > SHOULD BE < ' + replace[0] + ' >\n')
                
#find invalid block pointers
for entry in inode_csv:
    if int(entry[10]) < 11:
        for num in range(0, int(entry[10])):
            if int(entry[num+11],16) == 0 or int(entry[num+11],16) > TOTAL_BLOCKS:
                txtfile.write('INVALID BLOCK < ' + entry[num+11] + ' > IN INODE < ' + entry[0] + ' > ENTRY < ' + str(num) + ' >\n')
