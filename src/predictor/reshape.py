
#f_read = open("resdata.txt",'r');
#f_write = open("data3.txt",'w');
#
# i=0
# res = ''
#
# for i in range (0, 8010):
# 	f_read = open("resdata.txt", 'r');
# 	k = i
# 	while k != 0:
# 		temp = f_read.readline()[:-3]
# 		k -= 1
#
# 	for j in range(0, 29):
# 		res += f_read.readline()[:-1]
# 	res += f_read.readline()[:-2]
# 	f_write.write(res+'\n')
# 	res = ''
# 	f_read.close();
# f_write.close();

# File Writer

import collections

buffers = collections.deque([])
label = collections.deque([])

f_read = open("data.txt", 'r');
f = open("data5.txt", 'w')

while(True) :
    data = ""
    data = f_read.readline()[:-1]
    print(data)
    #if(data[-3]==6):
	#    continue

    data_f = data.split("/")
    #print(data_f[3])
    label_ = int(data_f[3])
    if(len(buffers) < 90) :
        #buffers.append(data_f[:3])
        label.append(label_)
        for i in range(3):
            buffers.append(str(data_f[i]))
    else :
        send_data = ""
        avg_label = 0
        for i in range(90):
            send_data += (str(buffers[i]) + " ")
            if(i%3==0):
                avg_label += label[int(i/3)]

        avg_label = round(avg_label / 30)
        buffers.popleft()
        buffers.popleft()
        buffers.popleft()
     #   buffers.append(str(data_f[:3]))
        for i in range(3):
            buffers.append(str(data_f[i]))
        label.popleft()
        label.append(label_)
        send_data += str(avg_label)
        f.write(send_data + "\n")
f_read.close()
f.close()
