#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

//These are big endian
void writeUint32(char *where, uint32_t toWrite) {
  where[3] = toWrite >> 0;
  where[2] = toWrite >> 8;
  where[1] = toWrite >> 16;
  where[0] = toWrite >> 24;
}

uint32_t readUint32(unsigned char *where) {
  return where[3] + (where[2]<<8) + (where[1]<<16) + (where[0]<<24);
}

void writeSecret(char* imageFilename, char* dataFilename) {
  //write image file into output
  ifstream image(imageFilename, ios::in | ios::binary | ios::ate);
  streampos imageSize = image.tellg();
  image.seekg(0, ios::beg);
  char* imageData = new char[(int)imageSize - 0xC]; //0xC for the IEND chunk
  image.read(imageData, (int)imageSize - 0xC);
  ofstream out("encoded.png", ios::out | ios::binary | ios::trunc);
  out.write(imageData, (int)imageSize - 0xC);
  delete[] imageData;
  image.close();

  //write data to output as ancillery chunk
  ifstream dataFile(dataFilename, ios::in | ios::binary | ios::ate);
  int dataSize = dataFile.tellg();
  dataFile.seekg(0, ios::beg);
  int filenameLength = strlen(dataFilename);
  //+13 for null terminator, length, type, and CRC
  char* data = new char[dataSize+filenameLength+13];
  writeUint32(data, dataSize+filenameLength+1);
  data[4] = 'x';
  data[5] = 't';
  data[6] = 'R';
  data[7] = 'a';
  for(int i=0; i<filenameLength; i++) {
    data[8+i] = dataFilename[i];
  }
  data[8+filenameLength] = '\0';
  dataFile.read(data+8+filenameLength+1, dataSize);
  dataFile.close();
  out.write(data, dataSize+filenameLength+13);
  delete[] data;
  
  //add final IEND chunk
  char end[] = {0, 0, 0, 0, 'I', 'E', 'N', 'D', 0xAE, 0x42, 0x60, 0x82};
  out.write(end, 0xC);

  out.close();
}

void readSecret(char* imageFilename) {
  ifstream image(imageFilename, ios::in | ios::binary | ios::ate);
  streampos imageSize = image.tellg();
  image.seekg(8, ios::beg);
  int filesFound = 0;
  unsigned char buf[8];
  while(image.tellg() < imageSize) {
    cout << "offset " << image.tellg() << " of filesize " << imageSize << endl;
    image.read((char*)buf, 8);
    uint32_t length = readUint32(buf);
    stringstream type;
    type << buf[4] << buf[5] << buf[6] << buf[7];
    cout << type.str() << endl;
    if(type.str() == "xtRa") {
      char *data = new char[length];
      image.read(data, length);
      image.seekg(4, ios::cur);
      string filename = "";
      int i=0;
      while(data[i] != '\0') {
        filename += data[i];
        i++;
      }
      i++;
      ofstream outFile(filename, ios::out | ios::binary);
      outFile.write(data+i, length-i);
      outFile.close();
      delete[] data;
    } else {
      image.seekg(length+4, ios::cur);
    }
  }
}

int main(int argc, char* argv[]) {
  if(argc < 3 || argc > 4) {
    cout << "Usage: hidepng <encode | decode | e | d> <image> [data]" << endl;
    return 1;
  }

  ifstream image((const char*)argv[2], ios::in | ios::binary | ios::ate);
  streampos imageSize = image.tellg();
  image.seekg(0, ios::beg);

  std::cout << argv[2] << " has filesize of " << imageSize << endl;
  unsigned char *buf = new unsigned char[100];
  image.read((char*)buf, 8);
  image.close();
  if(buf[0] != 0x89 ||
     buf[1] != 0x50 ||
     buf[2] != 0x4E ||
     buf[3] != 0x47 ||
     buf[4] != 0x0D ||
     buf[5] != 0x0A ||
     buf[6] != 0x1A ||
     buf[7] != 0x0A) {
    std::cout << argv[2] << " is not a PNG image" << endl;
    return 1;
  }

  if(strcmp(argv[1], "encode") == 0 ||
     strcmp(argv[1], "e") == 0) {
    if(argc >= 4) {
      writeSecret(argv[2], argv[3]);
    } else {
      cout << "Must have data file argument when encoding" << endl;
      return 1;
    }
  } else if(strcmp(argv[1], "decode") == 0 ||
            strcmp(argv[1], "d") == 0) {
    readSecret(argv[2]);
  } else {
    cout << "First argument must be encode or decode" << endl;
    return 1;
  }
  
  return 0;
}
