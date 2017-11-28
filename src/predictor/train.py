import tensorflow as tf
import numpy as np

########################### READ FILE #################################

FILE_PATH = "data5.txt"
DATA_SIZE = 30*3
LABEL_NUM = 3
batch_size = 50
CASE = 1500


dataSet = np.loadtxt(FILE_PATH, delimiter=' ', dtype=np.float32)
np.random.shuffle(dataSet)
x_data = dataSet[:, 0:90]
y_data = dataSet[:, [-1]]
print(x_data)
x_data_ = np.reshape(x_data, (-1, 3, 30))
#print (x_data_)
y_data = y_data.astype(int)
#y_data_ = tf.one_hot(y_data, 3)
#print(y_data_)

keep_prob = tf.placeholder("float")
keep_prob_hidden = tf.placeholder("float")

###########################

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


#####################################################################################################


######################### MODELING ############################
####################### PlaceHolder #######################

x = tf.placeholder(tf.float32, shape=[None, 3, 30])
y_ = tf.placeholder(tf.float32, shape=[None, LABEL_NUM])

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
y_conv = tf.nn.softmax(tf.matmul(h_fc1_drop, W_fc2) + b_fc2)


######################## TRAIN #############################


cross_entropy = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits=y_conv, labels=y_))
train_step = tf.train.AdamOptimizer(learning_rate=0.000001).minimize(cross_entropy)
correct_prediction = tf.equal(tf.argmax(y_conv,1), tf.argmax(y_,1))
accuracy = tf.reduce_mean(tf.cast(correct_prediction, "float"))

#x_image = tf.convert_to_tensor(x_data_)
#y_label = tf.convert_to_tensor(y_data)
saver = tf.train.Saver()


with tf.Session() as sess :
    sess.run(tf.global_variables_initializer())
    onehot_labels = tf.one_hot(y_data, LABEL_NUM)
    onehot_vals = sess.run(onehot_labels)
    for j in range(100000):
        avg_accuracy_val = 0
        batch_count = 0
        for i in range(0, CASE, batch_size) :
            batch_x = x_data_[i:i+batch_size,  :]
            batch_y = onehot_vals[i:i+batch_size, -1]
            cost_, accuracy_val = sess.run([train_step, accuracy], feed_dict={x: batch_x, y_: batch_y, keep_prob: 0.8, keep_prob_hidden:0.5})
            avg_accuracy_val += accuracy_val
            batch_count += 1
        else :
            avg_accuracy_val /=batch_count
            if(j%100==0) :
                save_path = saver.save(sess,'./tensorflow_live.ckpt')
                print('Epoch {}. Cost {}. Avg accurach {}'.format(j, cost_, avg_accuracy_val))

sess.close()

print("Model saved in file: ", save_path)




""" sess.run(train_step, feed_dict={x: batch_x, y_: batch_y, keep_prob: 0.5})
    if i%10 == 0 :
        train_accuracy = accuracy.eval(session = sess, feed_dict={x:batch_x, y_: batch_y, keep_prob: 1.0})
        print ("step %d, training accuracy %g" % (i, train_accuracy) ,
        train_step.run(session = sess, feed_dict={x: batch_x, y_: batch_y, keep_prob: 0.5}))
"""
