import os

def get_images(imagePath):
        for fileobj in os.listdir(imagePath):
            if fileobj=="image.txt":
                continue
            if os.path.isdir(os.path.join(imagePath, fileobj)):
    	        get_images(os.path.join(imagePath, fileobj))
            if os.path.isfile(os.path.join(imagePath, fileobj)):
                print("%s\n" % fileobj)
#                f.write(fileobj + '\n')
                if 'test' in fileobj:
                    f.write(fileobj.split('_')[0] + '_' + 'test ' + fileobj.split('_')[2].split('.')[0] + '\n')
                else:
                    f.write(fileobj.split('_')[0] + ' ' + fileobj.split("_")[1].split('.')[0] + '\n')

if __name__ == '__main__':
    with open("../test_images/image.txt", "w+") as f:
        get_images("../test_images")
