from PIL import Image
import glob
import os
folder = "changing_ior"
input_path = "/Users/mehdigorjian/Downloads/csce647/pr_10/" + folder + "/*.ppm"
output_path = "/Users/mehdigorjian/Downloads/csce647/pr_10/" + folder + "/jpg_" + folder
files = glob.glob(input_path)

for i, f in enumerate(files):
    img = Image.open(f)
    # _, f_name = os.path.split(f)
    # f_name = f_name.split('.')[0]
    # out = output_path + "/" + f_name + ".jpg"
    out = output_path + "/" + "output" + str(i) + ".jpg"

    print(out)
    img.save(out)
    # img.show()