# All units in mm
import math

# midpoint of pcb in kicad
PCB_MID = 100
# side length of square that could surround the pcb
PCB_DIM = 65


# (x, y)
kicad_positions = (
                    (100, 83.12), 
                    (114.75, 80.45),
                    (127.5, 90.5),
                    (129, 107),
                    (121.5, 112.5),
                    (109, 126),
                    (100, 112),
                    (91, 126),
                    (78.5, 112.5),
                    (71, 107),
                    (72.5, 90.5),
                    (83.35, 79.3),
                    (80.95, 94.95),
                    (100, 94.95),
                    (119.05, 94.95),
                   )

def real_location_mm(location):
    return (PCB_DIM/2) + location - PCB_MID


def qmk_transform(x, y):
    # on the pcb, x-location is mirrored about the y-axis compared to reality 
    transform_x = real_location_mm(PCB_MID * 2 - x) * 224 / PCB_DIM
    transform_y = real_location_mm(y) * 64 / PCB_DIM
    return (math.floor(transform_x), math.floor(transform_y))

    #// Underglow
    #{112  , 15}, {76 , 22}, {144, 22}, {0  , 28}, {75 , 29},
    #{149, 22}, {149, 23}, {149, 23}, {0  , 22}, {76 , 22},
    #{144, 22}, {0  , 28}, 
    #// Key matrix
    #{0  ,  0}, {75 ,  0}, {149,  0}, 

def print_g_led_config(qmk_transform_locations):
    underglow_num = 12
    key_matrix_printed = False
    print("// Underglow")
    i = 0
    for p in qmk_transform_locations:
        print('{' + str(p[0]) + ', ' + str(p[1]) + '}, ', end='')
        i = i+1
        if i >= underglow_num and not key_matrix_printed:
            print("\n// Key matrix")
            key_matrix_printed = True
    print()

def main():
    qmk_positions = []
    for position in kicad_positions:
        qmk_positions.append(qmk_transform(position[0], position[1]))
    print_g_led_config(qmk_positions)


main()


