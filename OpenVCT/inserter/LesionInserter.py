import numpy as np
import argparse

from readers.xml import Phantom as ph
from readers.xml import Inserter as le

class LesionInserter:
    def __init__(self):
        
        #Phantom
        self.phantom = None

    #def selectLesion(self, type, number, position):
        

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Lesion Inserter XML Reader")
    parser.add_argument('xml_input', type=str, help='Path to the XML file to read')

    args = parser.parse_args()
    if args.xml_input == None:
        exit

    else:
        print('Now, read xml and insert lesions')
       
