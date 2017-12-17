#include <X11/Xcursor/Xcursor.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// applies highlight and writes to new file
void highlight(const char* filename) {
    XcursorImages* imgs = XcursorFilenameLoadAllImages(filename);
    for (int i = 0; i < imgs->nimage; i++) {
        XcursorImage* img = imgs->images[i];

        // create a new pixels area 4 times the size for highlight circle
        XcursorPixel* p = img->pixels;
        XcursorUInt w = img->width;
        XcursorUInt h = img->height;
        XcursorPixel* hl = 
            calloc((w * 4) * (h * 4), sizeof(XcursorPixel));

        // x,y coords of where old img should go
        XcursorUInt x = w * 1.5; 
        XcursorUInt y = h * 1.5;

        // copy cursor into bigger space
        int j = 0;
        for (int row = 0; row < h; row++) {
            for (int col = 0; col < w; col++) {
                j = x + (w * 4 * y) + (w * 4 * row) + col;  
                hl[j] = p[(row * w) + col];
            }
        }

        // update the img data
        img->pixels = hl;
        img->size = img->size * 4;
        img->width = img->width * 4;
        img->height = img->height * 4;
        img->xhot = x + img->xhot;
        img->yhot = y + img->yhot;

        // add the highlight circle
        XcursorUInt rad = img->size / 2;
        XcursorUInt xmid = img->width / 2;
        XcursorUInt ymid = img->height / 2;
        for (int row = 0; row < img->height; row++) {
            for (int col = 0; col < img->width; col++) {
                x = abs(col - xmid);
                y = abs(row - ymid);
                // if pixel is fully transparant
                if ((hl[row * img->width + col] & 0xFF000000) == 0) {
                    // and in circle distance from center
                    float dist = sqrt(x*x + y*y);
                    if(dist < rad -1) {
                        hl[row * img->width + col] = 0x10666600;
                    } else if (dist < rad) { // outer rim slightly different
                        hl[row * img->width + col] = 0x66888800;
                    }
                }
            }
        }
    }

    // write highlighted cursor to file
    char output[255];
    strncpy(output, "/tmp/cursors/", 13);
    strncpy(output + 13, filename, 240);
    printf("Writing to: %s\n", output);
    XcursorFilenameSaveImages(output, imgs);

    // clean up
    for (int i = 0; i < imgs->nimage; i++) {
        XcursorImage* img = imgs->images[i];
        free(img->pixels);
    }
    XcursorImagesDestroy(imgs);
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        chdir(argv[1]);
    } else {
        printf("Usage: hlCursors [cursors directory]\n");
        printf("Example: hlCursors /usr/share/icons/DMZ-White/cursors\n");
        printf("Afterwards highlighted version are in /tmp/cursors\n");
        exit(0);
    }

    mkdir("/tmp/cursors", 0777);
    DIR* d = opendir(".");
    if (d) {
        struct dirent *dir;
        char buf[512];
        char out[512];
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                highlight(dir->d_name);
            }
            if (dir->d_type == DT_LNK) {
                int read = readlink(dir->d_name, buf, 511);
                buf[read] = 0;
                strncpy(out, "/tmp/cursors/", 13);
                strncpy(out + 13, dir->d_name, 495);
                symlink(buf, out);
                printf("Creating symlink: %s\n", out);
            }
        }
        closedir(d);
    }
}

