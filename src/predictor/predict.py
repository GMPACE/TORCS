from socket import *
import tensorflow as tf
import numpy as np

############################ SETTING Client ##############################

#client_socket = socket(AF_INET, SOCK_STREAM)
#client_socket.connect(('127.0.0.1', 8080))

############################ SETTING TCP/IP ##############################

PORT = 40000
server_socket = socket(AF_INET, SOCK_STREAM)
server_socket.bind(("", PORT))
server_socket.listen(5)

print("TCPServer Waiting for client on port " ,PORT)

########################### READ FILE #################################

DATA_SIZE = 30*3
LABEL_NUM = 3
batch_size = 50
CASE = 1500

keep_prob = tf.placeholder("float")
keep_prob_hidden = tf.placeholder("float")


def weight_variable(shape) :
    initial = tf.truncated_normal(shape, stddev=0.1)
    return tf.Variable(initial)

def bias_variable(shape) :
    initial = tf.constant(0.1, shape=shape)
    return tf.Variable(initial)

def conv2d(x, W) :
    return tf.nn.conv2d(x, W, strides=[1, 1, 1, 1], padding='SAME')

def max_pool_2x2(x) :
    return tf.nn.max_pool(x, ksize=[1, 2, 1, 1], strides=[1, 2, 1, 1], padding='SAME')

######################### MODELING ############################
####################### PlaceHolder #######################

x = tf.placeholder(tf.float32, shape=[1, 3, 30])
y_ = tf.placeholder(tf.float32, shape=[1, LABEL_NUM])

###################### Variables ########################

W = tf.Variable(tf.zeros([DATA_SIZE, LABEL_NUM]))
b = tf.Variable(tf.zeros([LABEL_NUM]))

################## initializer ##################

sess = tf.Session()


########################### First Layer ###############################

W_conv1 = weight_variable([3, 3, 1, 32])
b_conv1 = bias_variable([32])
x_image = tf.reshape(x, [-1,3,30,1])


h_conv1 = tf.nn.relu(conv2d(x_image, W_conv1) + b_conv1)
h_pool1 = max_pool_2x2(h_conv1)
print(h_conv1)
print(h_pool1)

h_drop1 = tf.nn.dropout(h_pool1, keep_prob)

########################### Second Layer ###############################

W_conv2 = weight_variable([3, 3, 32, 64])
b_conv2 = bias_variable([64])

h_conv2 = tf.nn.relu(conv2d(h_drop1, W_conv2) + b_conv2)
h_pool2 = max_pool_2x2(h_conv2)
print(h_conv2)
print(h_pool2)

h_drop2 = tf.nn.dropout(h_pool2, keep_prob)

########################### Third Layer ###############################

W_conv3 = weight_variable([3, 3, 64, 128])
b_conv3 = bias_variable([128])

h_conv3 = tf.nn.relu(conv2d(h_drop2, W_conv3) + b_conv3)
h_pool3 = max_pool_2x2(h_conv3)
print(h_conv3)
print(h_pool3)

h_drop3 = tf.nn.dropout(h_pool3, keep_prob)

########################### Full Connected Layer ###############################

W_fc1 = weight_variable([1*30*128, 256])
b_fc1 = bias_variable([256])

h_pool3_flat = tf.reshape(h_pool3, [-1, 1*30*128])

h_fc1 = tf.nn.relu(tf.matmul(h_pool3_flat, W_fc1) + b_fc1)
h_fc1_drop = tf.nn.dropout(h_fc1, keep_prob_hidden)

####################### SOFTMAX #############################

W_fc2 = weight_variable([256, LABEL_NUM])
b_fc2 = bias_variable([LABEL_NUM])
print(tf.matmul(h_fc1_drop, W_fc2) + b_fc2)
y_conv = tf.nn.softmax(tf.matmul(h_fc1_drop, W_fc2) + b_fc2)


saver = tf.train.Saver()

x_data = []

while 1:
    client_socket, address = server_socket.accept()
    print("I got a connection from ", address)
    while 1:
        data = str(client_socket.recv(1024))
        print(data)
        print(len(data))
        if (data == 'q' or data == 'Q'):
            client_socket.close()
            break
        else:
            data_ = data.split("#")
            data_np = np.array(data_[0:-1])
            x_data = np.reshape(data_np, (30, 3, -1))
            x_data = np.transpose(x_data)
            
        with tf.Session() as sess:
            sess.run(tf.global_variables_initializer())
            saver.restore(sess, 'tensorflow_live.ckpt')
            print(sess.run(y_conv, feed_dict={x: x_data, keep_prob: 1.0, keep_prob_hidden: 1.0}))
        data = str(client_socket.recv(1024))
        x_data = ""
        data = ""
        write(client_socket, "1", 1)
        
        

    sess.close()

server_socket.close()
print("SOCKET closed??END")
