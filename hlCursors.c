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
XcursorImages* highlight(const char* filename, const float scale, 
        const XcursorPixel hlColor, const XcursorPixel rimColor, const float rimWidth) {
    XcursorImages* imgs = XcursorFilenameLoadAllImages(filename);
    for (int i = 0; i < imgs->nimage; i++) {
        // create a new pixels area s times the size for highlight circle
        XcursorImage* img = imgs->images[i];
        XcursorUInt w = img->width;
        XcursorUInt h = img->height;
        XcursorImage* hl = XcursorImageCreate(w * scale, h * scale);

        XcursorPixel* p = img->pixels;
        XcursorPixel* hlp = hl->pixels;
        // needed, new image pixels are not always cleared!
        memset(hlp, 0, hl->width*hl->height*sizeof(XcursorPixel));

        // x,y coords of where top left of old img should be
        XcursorUInt x = w * (scale -1) / 2; 
        XcursorUInt y = h * (scale -1) / 2;

        // copy cursor image into center of the bigger space
        int j = 0;
        for (int row = 0; row < h; row++) {
            for (int col = 0; col < w; col++) {
                j = x + (w * scale * y) + (w * scale * row) + col;  
                hlp[j] = p[(row * w) + col];
            }
        }

        // set the xhot and yhot of the new img 
        hl->xhot = x + img->xhot;
        hl->yhot = y + img->yhot;
        hl->delay = img->delay;

        // add the highlight circle
        XcursorUInt rad = hl->size / 2;
        XcursorUInt xmid = hl->width / 2;
        XcursorUInt ymid = hl->height / 2;
        w = hl->width;
        h = hl->height;
        for (int row = 0; row < h; row++) {
            for (int col = 0; col < w; col++) {
                x = abs(col - xmid);
                y = abs(row - ymid);
                // if pixel is fully transparant
                if ((hlp[row * w + col] & 0xFF000000) == 0) {
                    // and in circle distance from center
                    float dist = sqrt(x*x + y*y);
                    if(dist < rad -rimWidth) {
                        // pixels are 32bit ARGB 
                        hlp[row * w + col] = hlColor;
                    } else if (dist < rad) { // outer rim slightly different
                        hlp[row * w + col] = rimColor;
                    }
                }
            }
        }
        imgs->images[i] = hl;
        XcursorImageDestroy(img);
    }
    return imgs;
}

int main(int argc, char* argv[]) {
    float scale = 3;
    XcursorPixel hlColor = 0x10666600;
    XcursorPixel rimColor = 0x66888800;
    float rimWidth = 1;
    if (argc >= 2) {
        chdir(argv[1]);
        if (argc >= 3) {
            scale = strtof(argv[2], NULL);
        }
        if (argc >= 4) {
            hlColor = strtol(argv[3], NULL, 16);
        }
        if (argc >= 5) {
            rimColor = strtol(argv[4], NULL, 16);
        }
        if (argc >= 6) {
            rimWidth = strtof(argv[5], NULL);
        }
    } else {
        printf("Usage: hlCursors [cursors directory] [scale] [hlColor] \
[rimColor] [rimWidth]\n\n");
        printf("Afterwards highlighted version are in /tmp/cursors\n\n");
        printf("The scale, color and width parameters are optional. \n\n");
        printf("Scale indicates the size of the image and defaults to 3. If \
the original cursor is 32x32, scale 2.5 would create a 80x80, putting the \
original image in the center and drawing the highlight around it\n\n");
        printf("Each color is a 32bit ARGB hex value, the highlight color \
defaults to: 10666600, and the rim color defaults to: 66888800\n\n");
        printf("Examples: hlCursors /usr/share/icons/DMZ-White/cursors\n");
        printf("          hlCursors /usr/share/icons/DMZ-Black/cursors 3 \
0x10666600 0x66888800\n");
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
                XcursorImages* imgs = 
                    highlight(dir->d_name, scale, hlColor, rimColor, rimWidth);

                // write highlighted cursor to file
                char output[255];
                strncpy(output, "/tmp/cursors/", 13);
                strncpy(output + 13, dir->d_name, 240);
                printf("Writing to: %s\n", output);
                XcursorFilenameSaveImages(output, imgs);
                printf("done");
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
