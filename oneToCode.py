#!/bin/python3

import sys
import os
import subprocess

darknet_command = ['darknet.exe', 'detector', 'test', 'test0715/cfg/obj.data', 'test0715/cfg/yolov3-tiny.cfg', 'test0715/cfg/weights/yolov3-tiny_258000.weights', '-save_labels']
translator_command = ['translate.exe']
sketch_command = ['python','D:/sketch-code-master/src/only_get_html.py', '--i-have-gui']

# darknet_command = './get_label'i
# translator_command = './get_fake_gui'
# sketch_command = '../sketch-code/src/only_get_html.py'

def main():
    args = sys.argv
    if len(args) < 2:
        print(f'Please add the path to the folder after {args[0]}')
        return

    path = args[1]
    for fname in os.listdir(path):
        if fname.endswith('.png') or fname.endswith('.jpg') :
            fname = path + '/' + fname
            print(f'Processing {fname}')
            subprocess.run(darknet_command + [fname])
            print(darknet_command + [fname])
            fname = fname[:-3] + 'txt'
            print(translator_command  + [fname])
            subprocess.run(translator_command  + [fname])
            fname =  fname[:-3]  + 'trans_txt'
            print(fname)
            subprocess.run(sketch_command  + [fname])
            fname = fname[:-9] + 'html'
            print(f'Generated {fname}')


if __name__ == '__main__': main()
