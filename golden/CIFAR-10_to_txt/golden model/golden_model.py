import torch
import torchvision
import torchvision.transforms as transforms
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import csv
import numpy as np
import json
import time
import matplotlib.pyplot as plt
from numba import jit
import math

if torch.cuda.is_available():
    device = torch.device('cuda')
else:
    device = torch.device('cpu')

transform = transforms.Compose([transforms.ToTensor(),
     transforms.Normalize((0.5, 0.5, 0.5), (0.5, 0.5, 0.5))])

trainset = torchvision.datasets.CIFAR10(root='./data', train=True,
                                        download=True, transform=transform)
trainloader = torch.utils.data.DataLoader(trainset, batch_size=4,
                                          shuffle=True, num_workers=2)

testset = torchvision.datasets.CIFAR10(root='./data', train=False,
                                       download=True, transform=transform)
testloader = torch.utils.data.DataLoader(testset, batch_size=4,
                                         shuffle=False, num_workers=2)

def readData(filename, total):
    with open(filename, newline='') as input:
        datas = csv.reader(input)
        temp = np.zeros(total)
        index = 0
        for data in datas:
            temp[index] = float(data[0])
            index += 1
        return temp
    
# Read weights
input_1300 = readData('parameters/activations/input_1300.csv', 3 * 32 * 32).reshape(3,32,32)
output_1300 = readData('parameters/activations/output_1300.csv', 10).reshape(10)
input_3108 = readData('parameters/activations/input_3108.csv', 3 * 32 * 32).reshape(3,32,32)
output_3108 = readData('parameters/activations/output_3108.csv', 10).reshape(10)
conv1_weight = readData('parameters/weights/conv1.weight.csv', 6 * 3 * 5 * 5).reshape(6,3,5,5)
conv2_weight = readData('parameters/weights/conv2.weight.csv', 16 * 6 * 5 * 5).reshape(16,6,5,5)
fc1_weight = readData('parameters/weights/fc1.weight.csv', 400 * 120).reshape(120,400)
fc2_weight = readData('parameters/weights/fc2.weight.csv', 120 * 84).reshape(84,120)
fc3_weight = readData('parameters/weights/fc3.weight.csv', 84 * 10).reshape(10,84)
fc3_bias = readData('parameters/weights/fc3.bias.csv', 10).reshape(10)

# Read scales
f = open('parameters/scale.json')
data = json.load(f)
input_scale = data["input_scale"]
conv1_output_scale = data["conv1_output_scale"]
conv2_output_scale = data["conv2_output_scale"] 
fc1_output_scale = data["fc1_output_scale"]
fc2_output_scale = data["fc2_output_scale"]
fc3_output_scale = data["fc3_output_scale"]
f.close()


@jit
def convolution_3D(activation, input_activation, weights, M, P, Q, C, R, S, bits):
    overflow = False
    for m in range(M):
        for p in range(P):
            for q in range(Q):
                for c in range(C):
                    for r in range(R):
                        for s in range(S):
                            activation[m][p][q] +=  input_activation[c][p + r][q + s] * weights[m][c][r][s]
                            if(activation[m][p][q] < -(2 ** (bits - 1))):
                                overflow = True
                                activation[m][p][q] = -(2 ** (bits - 1))
                            elif(((2 ** (bits - 1)) - 1) < activation[m][p][q]):
                                activation[m][p][q] = (2 ** (bits - 1)) - 1
                                overflow = True
    return overflow

@jit            
def RELU_3D(activation, M, P, Q):
    for m in range(M):
        for p in range(P):
            for q in range(Q):
                activation[m][p][q] = max(0, activation[m][p][q])
@jit
def max_pooling(activation, activation_pool, M, P, Q, I, J):
    for m in range(M):
        for p in range(P):
            for q in range(Q):
                for i in range(I):
                    for j in range(J):
                        activation_pool[m][p][q] = max(activation[m][p * 2 + i][q * 2 + j], activation_pool[m][p][q])

@jit
def fully_connect_layer(activation, input_activation, weights, M, H, bits):
    overflow = False
    for m in range(M):
        for h in range(H): 
            activation[m] += (input_activation[h] * weights[m][h])
            if(activation[m] < -(2 ** (bits - 1))):
                activation[m] = -(2 ** (bits - 1))
                overflow = True
            elif(((2 ** (bits - 1)) - 1) < activation[m]):
                activation[m] = (2 ** (bits - 1)) - 1
                overflow = True
    return overflow
@jit
def RELU(activation, M):
    for m in range(M):
        activation[m] = max(0, activation[m])


def DNN(input, bits, index):
    # input quantization
    input_quantized = (input * input_scale).astype(int)
    overflow = False
    
    filename = "input_quantized_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, input_quantized.astype(int).reshape(-1), fmt='%d')

    ## 1st convolution
    activation = np.zeros((6,28,28))
    layer1_activation = np.zeros((6,14,14))

    cur_overflow = convolution_3D(activation, input_quantized, conv1_weight, 6, 28, 28, 3, 5, 5, bits)
    overflow = (overflow or cur_overflow)
    
    filename = "conv1_activation_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, activation.astype(int).reshape(-1), fmt='%d')

    RELU_3D(activation, 6, 28, 28)
    max_pooling(activation, layer1_activation, 6, 14, 14, 2, 2)

    filename = "conv1_max_pool_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, layer1_activation.astype(int).reshape(-1), fmt='%d')
    

    layer1_activation_quantized = (layer1_activation / conv1_output_scale).astype(int)

    filename = "conv1_quantized_result_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, layer1_activation_quantized.astype(int).reshape(-1), fmt='%d')

    ## 2nd convolution
    activation = np.zeros((16,10,10))
    layer2_activation = np.zeros((16,5,5))
    
    cur_overflow = convolution_3D(activation, layer1_activation_quantized, conv2_weight, 16, 10, 10, 6, 5, 5, bits)
    overflow = (overflow or cur_overflow)
    
    filename = "conv2_activation_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, activation.astype(int).reshape(-1), fmt='%d')

    RELU_3D(activation, 16, 10, 10)
    max_pooling(activation, layer2_activation, 16, 5, 5, 2, 2)
    
    filename = "conv2_max_pool_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, layer2_activation.astype(int).reshape(-1), fmt='%d')

    layer2_activation_quantized = (layer2_activation / conv2_output_scale).astype(int)
  
    filename = "conv2_quantized_result_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, layer2_activation_quantized.astype(int).reshape(-1), fmt='%d')

    # flattern to 1D array
    layer2_activation_quantized = layer2_activation_quantized.flatten()
    
  
    filename = "fc1_input_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, layer2_activation_quantized.astype(int).reshape(-1), fmt='%d')

    # 1st fully connection
    fc1_activation = np.zeros(120)
    cur_overflow = fully_connect_layer(fc1_activation, layer2_activation_quantized, fc1_weight, 120, 400, bits)
    #plt.hist(fc1_activation)
    overflow = (overflow or cur_overflow)
    

    RELU(fc1_activation, 120)   
    filename = "fc1_activation_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, fc1_activation.astype(int).reshape(-1), fmt='%d')

    fc1_activation_quantized = (fc1_activation / fc1_output_scale).astype(int)

    filename = "fc1_quantized_result_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, fc1_activation_quantized.astype(int).reshape(-1), fmt='%d')
    
    # 2nd fully connection
    fc2_activation = np.zeros(84)
    cur_overflow = fully_connect_layer(fc2_activation, fc1_activation_quantized, fc2_weight, 84, 120, bits)
    #plt.hist(fc2_activation)
    overflow = (overflow or cur_overflow)
    
    RELU(fc2_activation, 84)   
    filename = "fc2_activation_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, fc2_activation.astype(int).reshape(-1), fmt='%d')
    fc2_activation_quantized = (fc2_activation / fc2_output_scale).astype(int)
    
    filename = "fc2_quantized_result_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, fc2_activation_quantized.astype(int).reshape(-1), fmt='%d')
    # 3rd fully connection
    fc3_activation = np.zeros(10)
    cur_overflow = fully_connect_layer(fc3_activation, fc2_activation_quantized, fc3_weight, 10, 84, bits)
    #plt.hist(fc3_activation)
    overflow = (overflow or cur_overflow)
    
    filename = "fc3_activation_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, fc3_activation.astype(int).reshape(-1), fmt='%d')
    for i in range(10):
        if(fc3_activation[i] + fc3_bias[i] < -(2 ** (bits - 1))):
            fc3_activation[i] = -(2 ** (bits - 1))
            overflow = True
        elif(((2 ** (bits - 1)) - 1) < fc3_activation[i] + fc3_bias[i]):
            fc3_activation[i] = (2 ** (bits - 1)) - 1
            overflow = True
        else:
            fc3_activation[i] += fc3_bias[i]
    
    filename = "fc3_bias_addition_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, fc3_activation.astype(int).reshape(-1), fmt='%d')

    fc3_activation_quantized = (fc3_activation / fc3_output_scale).astype(int)
    
    filename = "fc3_quantized_result_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, fc3_activation_quantized.astype(int).reshape(-1), fmt='%d')
    return fc3_activation_quantized, overflow


def generate_data(index, bits):
    input, label = trainset[index]
    predicted, overflow = DNN(input.numpy(), bits, index)
    filename = "data_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, input.cpu().data.numpy().reshape(-1),fmt='%f')
    filename = "label_" + str(index) + ".txt"
    np.savetxt('dataset/' + filename, predicted,fmt='%d')

bit = 32
start_time = time.time()
#for index in range(100):
generate_data(1, bit)
print('Time used: {} sec'.format(time.time() - start_time))

np.savetxt('conv1_weights.txt', conv1_weight.astype(int).reshape(-1),fmt='%d')
np.savetxt('conv2_weights.txt', conv2_weight.astype(int).reshape(-1),fmt='%d')
np.savetxt('fc1_weights.txt', fc1_weight.astype(int).reshape(-1),fmt='%d')
np.savetxt('fc2_weights.txt', fc2_weight.astype(int).reshape(-1),fmt='%d')
np.savetxt('fc3_weights.txt', fc3_weight.astype(int).reshape(-1),fmt='%d')
np.savetxt('fc3_bias.txt', fc3_bias.astype(int).reshape(-1),fmt='%d')
