#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "maps.h"
#define BASE_ADDRESS 100

typedef enum {
    IDLE,
    LEFT,
    RIGHT,
    SELECT,
    DOWN,
    UP
} state_t;

state_t check_keypress() {
    char c[8];
    state_t ret = IDLE;
    printf("\n\nPress key: ");
    scanf("%s", c);
    if (c[0]=='l' || c[0]=='L')
        ret=LEFT;
    else if (c[0]=='r' || c[0]=='R')
        ret=RIGHT;
    else if (c[0]=='u' || c[0]=='U')
        ret=UP;
    else if (c[0]=='d' || c[0]=='D')
        ret=DOWN;
    else if (c[0]=='s' || c[0]=='S')
        ret=SELECT;
    else
        ret=IDLE;

    return ret;
}

int move_cursor(int cursor, state_t key_pressed) {

    bool right_edge, left_edge, up_edge, down_edge;
/* ------------- testing calculation ---------------
    int m = get_mem_loc_from_cursor(cursor);
    printf("Position from get_mem_loc_from_cursor: %d\n", m);
*/
    int pos = get_mem_loc_from_cursor(cursor);

    left_edge = (pos%10==0) ? true : false;
    right_edge = (pos%10==9) ? true : false;
    down_edge = (pos>=90) ? true : false;
    up_edge = (pos<=9) ? true : false;

    if (key_pressed==LEFT)
        cursor = (left_edge) ? cursor : cursor-4;
    else if (key_pressed==RIGHT)
        cursor = (right_edge) ? cursor : cursor+4;
    else if (key_pressed==UP)
        cursor = (up_edge) ? cursor : cursor-160;
    else if (key_pressed==DOWN)
        cursor = (down_edge) ? cursor : cursor+160;
    else if (key_pressed==SELECT)
        cursor=cursor;
    return cursor;
}

int get_cursor_from_mem(int mem_location) {
    int cursor_x, cursor_y;
    cursor_y=mem_location/10;
    cursor_x=mem_location%10;
    /* -- optional - just for testing --
    printf("\nCursor y: %d", cursor_y);
    printf("\nCursor x: %d\n", cursor_x);
//    */
    return BASE_ADDRESS + cursor_y*160 + cursor_x*4;
}

int get_mem_loc_from_cursor(int cursor_pos) {
    int mem_x, mem_y;
    cursor_pos-=BASE_ADDRESS;
    mem_y=cursor_pos/160;
    mem_x=cursor_pos%40/4;
    /* -- optional - just for testing --
    printf("\nCursor y: %d", mem_y);
    printf("\nCursor x: %d\n", mem_x);
    */
    return mem_y*10 + mem_x;
}

void print_matrix(char c[]) {

	int i=0;
	for (i=0; i<100; i++) {
		if (i%10==0) {
            if (i!=0) printf("\n");
            printf("%c ", c[i]);
        }
		else {
            printf("%c ", c[i]);
		}
	}
}

void upisi_u_mapu(char *mapa, char c, int pos) {
    if (c!='#')
        mapa[pos]=c;
}

void remove_edges(char* m, int p) {
    bool horizontal, vertical, single;
    bool right_edge, left_edge, up_edge, down_edge;
    int upper, down, left, right;

    int ship_size=1;

    bool left_up, left_down, right_down, right_up;

    left_edge = (p%10==0) ? true : false;
    right_edge = (p%10==9) ? true : false;
    down_edge = (p>90) ? true : false;
    up_edge = (p<=9) ? true : false;

    left_up = left_edge && up_edge;
    left_down = left_edge && down_edge;
    right_down = right_edge && down_edge;
    right_up = right_edge && up_edge;

//    printf("\nUSAO\n");
//    (down_edge) ? printf("\nTRUE\n") : printf("\nFALSE\n");

    if (up_edge) {
        vertical = ((m[p]=='1' || m[p]=='*') && (m[p+10]=='1' || m[p+10]=='*')) ? true : false;
    } else if (down_edge) {
        vertical = ((m[p]=='1' || m[p]=='*') && (m[p-10]=='1' || m[p-10]=='*')) ? true : false;
    } else if (!(up_edge || down_edge)){
        vertical = ((m[p]=='1' || m[p]=='*') && ((m[p+10]=='1' || m[p+10]=='*') || (m[p-10]=='1' || m[p-10]=='*'))) ? true : false;
    }

    if (left_edge) {
        horizontal = ((m[p]=='1' || m[p]=='*') && (m[p+1]=='1' || m[p+1]=='*')) ? true : false;
    } else if (right_edge) {
        horizontal = ((m[p]=='1' || m[p]=='*') && (m[p-1]=='1' || m[p-1]=='*')) ? true : false;
    } else if (!(left_edge || right_edge)) {
        horizontal = ((m[p]=='1' || m[p]=='*') && ((m[p+1]=='1' || m[p+1]=='*') || (m[p-1]=='1' || m[p-1]=='*'))) ? true : false;
    }

    single = (!horizontal) && (!vertical);

    if (vertical) printf("\nVERTIKALNO\n");
//    else printf("NOO");

    if (horizontal) printf("\nHORIZONTALNO\n");
//    else printf("NOO");

    if (single) printf("\nMALA PODMORNICA\n");
    else printf("VELIKA PODMORNICA (>1)");


    int temp = p;
    if (horizontal) {
        if (!right_edge) {
            while (m[temp]=='1' || m[temp]=='*') {
                right=temp;
                temp+=1;
            }
        }
        else {
            right=temp;
        }
        temp=p;
        if (!left_edge) {
            while (m[temp]=='1' || m[temp]=='*') {
                left=temp;
                temp-=1;
            }
        }
        else {
            left=temp;
        }

    printf("\nNajlevlji je %d, a najdesnji %d\n", left, right);
    ship_size = right-left+1;
    printf("\nDuzina podmornice: %d\n", ship_size);
    }
    temp = p;
    if (vertical) {
        if (!down_edge) {
            while (m[temp]=='1' || m[temp]=='*') {
                upper=temp;
                temp-=10;
            }
        }
        else {
            down=temp;
        }

        temp=p;
        if (!up_edge) {
            while (m[temp]=='1' || m[temp]=='*') {
                down=temp;
                temp+=10;
            }
        }
        else {
            upper=temp;
        }


    printf("\nGornji je %d, a donji je %d\n", upper, down);
    ship_size = (down-upper)/10+1;
    printf("\nDuzina podmornice: %d\n", ship_size);
    }

    temp=p;
    if (single && m[p]=='1') {
        if (!up_edge && !left_edge && !right_edge && !down_edge) {
            m[p-11]='k';
            m[p-10]='k';
            m[p-9]='k';
            m[p-1]='k';
            m[p+1]='k';
            m[p+9]='k';
            m[p+10]='k';
            m[p+11]='k';
        }
        if (left_up) {
            m[p+1]='k';
            m[p+10]='k';
            m[p+11]='k';
        }
        if (right_up) {
            m[p-1]='k';
            m[p+10]='k';
            m[p+9]='k';
        }
        if (left_down) {
            m[p-10]='k';
            m[p-9]='k';
            m[p+1]='k';
        }
        if (right_down) {
            m[p-11]='k';
            m[p-10]='k';
            m[p-1]='k';
        }
        if (left_edge && !up_edge && !down_edge) {
            m[p+1]='k';
            m[p+10]='k';
            m[p+11]='k';
            m[p-10]='k';
            m[p-9]='k';
        }
        if (right_edge && !up_edge && !down_edge) {
            m[p-1]='k';
            m[p+10]='k';
            m[p+9]='k';
            m[p-11]='k';
            m[p-10]='k';
        }
        if (up_edge && !right_edge && !left_edge) {
            m[p+1]='k';
            m[p+10]='k';
            m[p+11]='k';
            m[p-1]='k';
            m[p+9]='k';
        }
        if (down_edge && !left_edge && !right_edge) {
            m[p+1]='k';
            m[p-1]='k';
            m[p-9]='k';
            m[p-11]='k';
            m[p-10]='k';
        }
    }
    m[p]='*';

    bool completed = true;
    if (vertical) {
        completed = true;
        int t = upper;
        for (t=upper; t<=down; t+=10) {
            if(m[t]=='1') {
                completed = false;
            }
        }
        completed ? printf("Vertikalni porusen") : printf("Vertikalni nije porusen");

        left_edge = (upper%10==0) ? true : false;
        right_edge = (upper%10==9) ? true : false;
        down_edge = (down>90) ? true : false;
        up_edge = (upper<=9) ? true : false;

        left_up = left_edge && up_edge;
        left_down = left_edge && down_edge;
        right_down = right_edge && down_edge;
        right_up = right_edge && up_edge;

        if (completed) {
            if (!up_edge && !left_edge && !right_edge && !down_edge) {
                m[upper-1]='k';
                m[upper-11]='k';
                m[upper-10]='k';
                m[upper-9]='k';
                m[upper+1]='k';
                m[down+10]='k';
                m[down+11]='k';
                m[down+9]='k';
                m[down+1]='k';
                m[down-1]='k';

                for (t=upper; t<down; t+=10) {
                    m[t+1]='k';
                    m[t-1]='k';
                }
            }
            if (left_up) {
                m[upper+1]='k';
                m[down+1]='k';
                m[down+10]='k';
                m[down+11]='k';
                for (t=upper; t<down; t+=10) {
                    m[t+1]='k';
                }
            }
            if (right_up) {
                m[upper-1]='k';
                m[down+10]='k';
                m[down+9]='k';
                m[down-1]='k';
                for (t=upper; t<down; t+=10) {
                    m[t-1]='k';
                }
            }
            if (left_down) {
                m[upper-10]='k';
                m[upper-9]='k';
                m[upper+1]='k';
                m[down+1]='k';
                for (t=upper; t<down; t+=10) {
                    m[t+1]='k';
                }
            }
            if (right_down) {
                m[upper-11]='k';
                m[upper-10]='k';
                m[upper-1]='k';
                m[down-1]='k';
            }
            if (left_edge && !up_edge && !down_edge) {
                m[upper+1]='k';
                m[upper-10]='k';
                m[upper-9]='k';
                m[down+10]='k';
                m[down+11]='k';
                m[down+1]='k';
                for (t=upper; t<down; t+=10) {
                    m[t+1]='k';
                }
            }
            if (right_edge && !up_edge && !down_edge) {
                m[upper-1]='k';
                m[upper-11]='k';
                m[upper-10]='k';
                m[down-1]='k';
                m[down+10]='k';
                m[down+9]='k';
                for (t=upper; t<down; t+=10) {
                    m[t-1]='k';
                }
            }
            if (up_edge && !right_edge && !left_edge) {
                m[upper+1]='k';
                m[upper-1]='k';
                m[down+1]='k';
                m[down-1]='k';
                m[down+10]='k';
                m[down+11]='k';
                m[down+9]='k';
                for (t=upper; t<down; t+=10) {
                    m[t+1]='k';
                    m[t-1]='k';
                }
            }
            if (down_edge && !left_edge && !right_edge) {
                m[upper+1]='k';
                m[upper-1]='k';
                m[upper-9]='k';
                m[upper-11]='k';
                m[upper-10]='k';
                m[down+1]='k';
                m[down-1]='k';
                for (t=upper; t<down; t+=10) {
                    m[t+1]='k';
                    m[t-1]='k';
                }
            }
        }
    }

    if (horizontal) {
        completed = true;
        int t = left;
        for (t; t<=right; t++) {
            if (m[t]=='1') {
                completed = false;
            }
        }
        completed ? printf("Horizontalni porusen") : printf("Horizontalni nije porusen");

        if (completed) {
            left_edge = (left%10==0) ? true : false;
            right_edge = (right%10==9) ? true : false;
            down_edge = (left>90) ? true : false;
            up_edge = (left<=9) ? true : false;

            left_up = left_edge && up_edge;
            left_down = left_edge && down_edge;
            right_down = right_edge && down_edge;
            right_up = right_edge && up_edge;

            if (!up_edge && !left_edge && !right_edge && !down_edge) {
                m[left-1]='k';
                m[left-11]='k';
                m[left-10]='k';
                m[left+9]='k';
                m[left+10]='k';
                m[right-10]='k';
                m[right-9]='k';
                m[right+1]='k';
                m[right+10]='k';
                m[right+11]='k';

                for (t=left+1; t<right; t++) {
                    m[t+10]='k';
                    m[t-10]='k';
                }
            }
            if (left_up) {
                m[right+1]='k';
                m[right+10]='k';
                m[right+11]='k';
                m[left+10]='k';

                for (t=left+1; t<right; t++) {
                    m[t+10]='k';
                }
            }
            if (right_up) {
                m[left-1]='k';
                m[left+10]='k';
                m[left+9]='k';
                m[right+10]='k';

                for (t=left+1; t<right; t++) {
                    m[t+10]='k';
                }
            }
            if (left_down) {
                m[right-10]='k';
                m[right-9]='k';
                m[right+1]='k';
                m[left-10]='k';

                for (t=left+1; t<right; t++) {
                    m[t-10]='k';
                }
            }
            if (right_down) {
                m[left-11]='k';
                m[left-10]='k';
                m[left-1]='k';
                m[right-10]='k';

                for (t=left+1; t<right; t++) {
                    m[t-10]='k';
                }

            }
            if (left_edge && !up_edge && !down_edge) {
                m[left-10]='k';
                m[left+10]='k';
                m[right-10]='k';
                m[right+10]='k';
                m[right+1]='k';
                m[right+11]='k';
                m[right-9]='k';

                for (t=left+1; t<right; t++) {
                    m[t+10]='k';
                    m[t-10]='k';
                }
            }
            if (right_edge && !up_edge && !down_edge) {
                m[left-1]='k';
                m[left+9]='k';
                m[left-11]='k';
                m[left-10]='k';
                m[left+10]='k';
                m[right-10]='k';
                m[right+10]='k';

                for (t=left+1; t<right; t++) {
                    m[t+10]='k';
                    m[t-10]='k';
                }
            }
            if (up_edge && !right_edge && !left_edge) {
                m[left-1]='k';
                m[left+9]='k';
                m[left+10]='k';
                m[right+10]='k';
                m[right+1]='k';
                m[right+11]='k';

                for (t=left; t<right; t++) {
                    m[t+10]='k';
                }
            }
            if (down_edge && !left_edge && !right_edge) {
                m[left-1]='k';
                m[left-11]='k';
                m[left-10]='k';
                m[right+1]='k';
                m[right-9]='k';
                m[right-10]='k';

                for (t=left+1; t<right; t++) {
                    m[t-10]='k';
                }
            }
        }
    }
    printf("\n");
    print_matrix(m);
}

int main()
{
    char *mapa = map1;
    char *m = mask;
    char *p = mask;

    printf("\n\n");

    print_matrix(map0);
    printf("\n\n");

    remove_edges(map0, 3);
    printf("\n\n");

    remove_edges(map0, 4);
    printf("\n\n");

    remove_edges(map0, 5);
    printf("\n\n");

    remove_edges(map0, 29);
    printf("\n\n");

    remove_edges(map0, 39);
    printf("\n\n");

    remove_edges(map0, 80);
    printf("\n\n");

    remove_edges(map0, 25);
    printf("\n\n");

    remove_edges(map0, 90);
    printf("\n\n");

    remove_edges(map0, 26);
    printf("\n\n");

//    remove_edges(map0, 88);
//    printf("\n\n");


    //print_matrix(m);

    int completed_ships = 0;
    bool on_turn = true;
    int cnext=BASE_ADDRESS, ctemp;
    state_t key = IDLE;
/*
    while(completed_ships<20) {
        on_turn=true;
        key=IDLE;
        while (on_turn) {
            while (key!=SELECT){
                key=check_keypress();
                cnext=move_cursor(cnext, key);
                m[get_mem_loc_from_cursor(cnext)]=' ';
        ///        printf("\nTest case - STARTING_ADDRESS %d\n", BASE_ADDRESS);
        ///        printf("\n\t%d - Actual position in array \n", get_mem_loc_from_cursor(cnext));
        ///        printf("\n\t%d - Actual position on screen\n", cnext);
                print_matrix(m);
                printf("\n-------------------------------------------------------------------------------\n");
                upisi_u_mapu(m, '#', get_mem_loc_from_cursor('#'));
            }
            key=IDLE;
            int x = get_mem_loc_from_cursor(cnext);
            if (mapa[x]=='0') {
                upisi_u_mapu(mask,'0',x);
                upisi_u_mapu(mapa, '0', x);
                on_turn=false;
            } else if (mapa[x]=='1') {
                mask[x]='X';
                mapa[x]='X';
                completed_ships++;
                if (completed_ships==20) break;
            }
            print_matrix(mask);
        }

    }
*/
/*
    remove_edges(map2, 9);
    print_matrix(map2);
    printf("\n\n");

    remove_edges(map2, 64);
    print_matrix(map2);
    printf("\n\n");

    remove_edges(map2, 83);
    print_matrix(map2);
    printf("\n\n");
*/
//    printf("\n\n");
//    print_matrix(map2);
//    remove_edges(map2, 4);

//   printf("\n\n");
//    remove_edges(map2, 9);
//    print_matrix(map2);
/*
    printf("\nPrint matrix from map()\n\n");
    char *c = map();
    print_matrix(c);

    printf("\n\n");
    printf("Print changed matrix\n\n");
    c[get_mem_loc_from_cursor(cnext)]='m';
    print_matrix(c);
*/
/// --------------------------------------- Uncomment for test ---------------------------------------

/*
    printf("\n*******************************************************************************");

    printf("\nTest case - starting position - set_cursor(%d)\n", BASE_ADDRESS);

    int mem = 19;
    printf("\nElement on position map1[%d]", mem);
    printf("\nPosition in array: %d, position on the screen: %d\n", mem, get_cursor_from_mem(mem));

    int cursor = 296;
    printf("\nElement on set_cursor(%d)", cursor);
    printf("\nPosition on the screen: %d, position in array: %d\n", cursor, get_mem_loc_from_cursor(cursor));

    printf("\n*******************************************************************************\n");
*/
    return 0;
}
