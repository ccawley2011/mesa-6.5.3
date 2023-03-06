/* vgears. */

/*
 * Spinning gears demo for Linux SVGA/Mesa interface in 32K color mode.
 *
 * Compile with:  gcc vgears.c -I../include -L../lib -lMesaGL -lX11 -lXext
 *   -lvga -lm -o vgears
 *
 * Brian Paul, January 1996
 */

#include <stdio.h>
#include <stdlib.h>
#include "GL/osmesa.h"
#include <math.h>
#include <string.h>
#include <swis.h>
#include <kernel.h>

#define WIDTH 128
#define HEIGHT 128
#define HEADER 60
#define BPP 8
#define LOG2BPP 3
#define UNITS 4

_kernel_swi_regs	regs;
_kernel_oserror		error;

OSMesaContext ctx;

   unsigned char *buffer;
   int	window_block[23] = {0, 256, 256, 256+(WIDTH*UNITS), 256+(HEIGHT*UNITS), 0, 0, -1, 0x87040012, 0xff070207, 0x000c0103, 0, -HEIGHT*UNITS, WIDTH*UNITS, 0, 0x2700003d, 0x00002000, 1, 0, 000, 0, 0, 0};
	char		task_name[16] = "Gears";
	unsigned int	task_handle;
   int	icon_block[9] = {0, 0, -HEIGHT*UNITS, WIDTH*UNITS, 0, 0x2102, 0, 0, 0};
   int	message[64];
   int	workspace[64];
   char	sprite_name[12] = "display";

typedef struct {
   char title[12];
   GLubyte title_fg;
   GLubyte title_bg;
   GLubyte work_fg;
   GLubyte work_bg;
   GLuint width;
   GLuint height;
   GLuint vgap;
} menu_header;

typedef struct {
   GLuint item_flags;
   void *handle;
   GLuint icon_flags;
   char *text;
   char *valid;
   int length;
} menu_item;

static struct {
    menu_header block;
    menu_item items[2];
} menu_block = {
   { "Gears", 7, 2, 7, 0, (17+1)*16, 44, 0 },
   {
      { 3,     (void *)-1, 0x07000101, "Lighting", NULL, 8 },
      { 128,   (void *)-1, 0x07000101, "Quit", NULL, 4 }
   }
};

#define lighting_enabled (menu_block.items[0].item_flags & 1)


/*
 * Draw a gear wheel.  You'll probably want to call this function when
 * building a display list since we do a lot of trig here.
 *
 * Input:  inner_radius - radius of hole at center
 *         outer_radius - radius at center of teeth
 *         width - width of gear
 *         teeth - number of teeth
 *         tooth_depth - depth of tooth
 */
static void gear( GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
		  GLint teeth, GLfloat tooth_depth, const GLfloat col[4] )
{
   GLint i;
   GLfloat r0, r1, r2;
   GLfloat angle, da;
   GLfloat u, v, len;

   r0 = inner_radius;
   r1 = outer_radius - tooth_depth/2.0;
   r2 = outer_radius + tooth_depth/2.0;

   da = 2.0*M_PI / teeth / 4.0;

   if ( lighting_enabled )
      glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col );
   else
      glColor4f( col[0], col[1], col[2], col[3] );

   glShadeModel( GL_FLAT );

   glNormal3f( 0.0, 0.0, 1.0 );

   /* draw front face */
   glBegin( GL_QUAD_STRIP );
   for (i=0;i<=teeth;i++) {
      angle = i * 2.0*M_PI / teeth;
      glVertex3f( r0*cos(angle), r0*sin(angle), width*0.5 );
      glVertex3f( r1*cos(angle), r1*sin(angle), width*0.5 );
      glVertex3f( r0*cos(angle), r0*sin(angle), width*0.5 );
      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5 );
   }
   glEnd();

   /* draw front sides of teeth */
   glBegin( GL_QUADS );
   da = 2.0*M_PI / teeth / 4.0;
   for (i=0;i<teeth;i++) {
      angle = i * 2.0*M_PI / teeth;

      glVertex3f( r1*cos(angle),      r1*sin(angle),      width*0.5 );
      glVertex3f( r2*cos(angle+da),   r2*sin(angle+da),   width*0.5 );
      glVertex3f( r2*cos(angle+2*da), r2*sin(angle+2*da), width*0.5 );
      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5 );
   }
   glEnd();


   glNormal3f( 0.0, 0.0, -1.0 );

   /* draw back face */
   glBegin( GL_QUAD_STRIP );
   for (i=0;i<=teeth;i++) {
      angle = i * 2.0*M_PI / teeth;
      glVertex3f( r1*cos(angle), r1*sin(angle), -width*0.5 );
      glVertex3f( r0*cos(angle), r0*sin(angle), -width*0.5 );
      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5 );
      glVertex3f( r0*cos(angle), r0*sin(angle), -width*0.5 );
   }
   glEnd();

   /* draw back sides of teeth */
   glBegin( GL_QUADS );
   da = 2.0*M_PI / teeth / 4.0;
   for (i=0;i<teeth;i++) {
      angle = i * 2.0*M_PI / teeth;

      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5 );
      glVertex3f( r2*cos(angle+2*da), r2*sin(angle+2*da), -width*0.5 );
      glVertex3f( r2*cos(angle+da),   r2*sin(angle+da),   -width*0.5 );
      glVertex3f( r1*cos(angle),      r1*sin(angle),      -width*0.5 );
   }
   glEnd();

   if ( !lighting_enabled )
      glColor4f( col[0] * 0.5f, col[1] * 0.5f, col[2] * 0.5f, col[3] * 0.5f );

   /* draw outward faces of teeth */
   glBegin( GL_QUAD_STRIP );
   for (i=0;i<teeth;i++) {
      angle = i * 2.0*M_PI / teeth;

      glVertex3f( r1*cos(angle),      r1*sin(angle),       width*0.5 );
      glVertex3f( r1*cos(angle),      r1*sin(angle),      -width*0.5 );
      u = r2*cos(angle+da) - r1*cos(angle);
      v = r2*sin(angle+da) - r1*sin(angle);
      len = sqrt( u*u + v*v );
      u /= len;
      v /= len;
      glNormal3f( v, -u, 0.0 );
      glVertex3f( r2*cos(angle+da),   r2*sin(angle+da),    width*0.5 );
      glVertex3f( r2*cos(angle+da),   r2*sin(angle+da),   -width*0.5 );
      glNormal3f( cos(angle), sin(angle), 0.0 );
      glVertex3f( r2*cos(angle+2*da), r2*sin(angle+2*da),  width*0.5 );
      glVertex3f( r2*cos(angle+2*da), r2*sin(angle+2*da), -width*0.5 );
      u = r1*cos(angle+3*da) - r2*cos(angle+2*da);
      v = r1*sin(angle+3*da) - r2*sin(angle+2*da);
      glNormal3f( v, -u, 0.0 );
      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da),  width*0.5 );
      glVertex3f( r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5 );
      glNormal3f( cos(angle), sin(angle), 0.0 );
   }

   glVertex3f( r1*cos(0), r1*sin(0), width*0.5 );
   glVertex3f( r1*cos(0), r1*sin(0), -width*0.5 );

   glEnd();


   glShadeModel( GL_SMOOTH );

   /* draw inside radius cylinder */
   glBegin( GL_QUAD_STRIP );
   for (i=0;i<=teeth;i++) {
      angle = i * 2.0*M_PI / teeth;
      glNormal3f( -cos(angle), -sin(angle), 0.0 );
      glVertex3f( r0*cos(angle), r0*sin(angle), -width*0.5 );
      glVertex3f( r0*cos(angle), r0*sin(angle), width*0.5 );
   }
   glEnd();

}


static GLfloat view_rotx=20.0, view_roty=30.0, view_rotz=0.0;
static GLint gears, gear1, gear2, gear3;
static GLfloat angle = 0.0;

static GLuint limit;
static GLuint count = 1;

static void draw( void )
{
   angle += 2.0;

   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

   glPushMatrix();
   glRotatef( view_rotx, 1.0, 0.0, 0.0 );
   glRotatef( view_roty, 0.0, 1.0, 0.0 );
   glRotatef( view_rotz, 0.0, 0.0, 1.0 );

   glPushMatrix();
   glTranslatef( -3.0, -2.0, 0.0 );
   glRotatef( angle, 0.0, 0.0, 1.0 );
   glCallList(gear1);
   glPopMatrix();

   glPushMatrix();
   glTranslatef( 3.1, -2.0, 0.0 );
   glRotatef( -2.0*angle-9.0, 0.0, 0.0, 1.0 );
   glCallList(gear2);
   glPopMatrix();

   glPushMatrix();
   glTranslatef( -3.1, 4.2, 0.0 );
   glRotatef( -2.0*angle-25.0, 0.0, 0.0, 1.0 );
   glCallList(gear3);
   glPopMatrix();

   glPopMatrix();

}


static void init( void )
{
   static GLfloat pos[4] = {5.0, 5.0, 10.0, 1.0 };
   static GLfloat red[4] = {0.8, 0.1, 0.0, 1.0 };
   static GLfloat green[4] = {0.0, 0.8, 0.2, 1.0 };
   static GLfloat blue[4] = {0.2, 0.2, 1.0, 1.0 };

   GLfloat w = (float) WIDTH / (float) HEIGHT;
   GLfloat h = 1.0;

   glEnable( GL_CULL_FACE );
   glEnable( GL_DEPTH_TEST );

   if ( lighting_enabled ) {
      glEnable( GL_LIGHTING );
      glEnable( GL_LIGHT0 );
      glLightfv( GL_LIGHT0, GL_POSITION, pos );
   } else {
      glDisable( GL_LIGHTING );
   }

   /* make the gears */
   gears = glGenLists(3);

   gear1 = gears + 0;
   glNewList(gear1, GL_COMPILE);
   gear( 1.0, 4.0, 1.0, 20, 0.7, red );
   glEndList();

   gear2 = gears + 1;
   glNewList(gear2, GL_COMPILE);
   gear( 0.5, 2.0, 2.0, 10, 0.7, green );
   glEndList();

   gear3 = gears + 2;
   glNewList(gear3, GL_COMPILE);
   gear( 1.3, 2.0, 0.5, 10, 0.7, blue );
   glEndList();

   glEnable( GL_NORMALIZE );


   glViewport( 0, 0, WIDTH, HEIGHT );
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   if (WIDTH>HEIGHT) {
      GLfloat w = (GLfloat) WIDTH / (GLfloat) HEIGHT;
      glFrustum( -w, w, -1.0, 1.0, 5.0, 60.0 );
   }
   else {
      GLfloat h = (GLfloat) HEIGHT / (GLfloat) WIDTH;
      glFrustum( -1.0, 1.0, -h, h, 5.0, 60.0 );
   }

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef( 0.0, 0.0, -40.0 );

/* Added by David Boddie (Thu 18th March 1999) */
  glDisable(GL_DITHER);

}

static void deinit( void )
{
   glDeleteLists(gears, 3);
}

void clear_screen(void *buffer)
{
	int	i;
	GLuint *ptr4;

	ptr4 = (GLuint *) buffer;

	for (i=0;i < (WIDTH*HEIGHT*BPP/32); i++)
	{
		ptr4[i] = 0;
	}
}

void setup( void )
{

	/* Write the window title */
	strcpy((char *)&window_block[19], "Gears");

   /* Create a single buffered RGB8-mode context */
   ctx = OSMesaCreateContext( OSMESA_RGBA, NULL );
   if (ctx==NULL) exit(0);

   /* Allocate the image buffer */
	buffer = malloc( (size_t) WIDTH * HEIGHT * 4 + HEADER );

   /* Initialise a sprite area */

	*(unsigned int *)buffer		= WIDTH * HEIGHT * 4 + HEADER;
	*(unsigned int *)(buffer + 8)	= 0;
	*(unsigned int *)(buffer + 8)	= 16;
	*(unsigned int *)(buffer + 8)	= 16;

	regs.r[0] = 256+9;
	regs.r[1] = (unsigned int)buffer;
   _kernel_swi(OS_SpriteOp, &regs, &regs);

	regs.r[0] = 256+15;
	regs.r[1] = (unsigned int)buffer;
	regs.r[2] = (unsigned int)&sprite_name;
	regs.r[3] = 0;
	regs.r[4] = WIDTH;
	regs.r[5] = HEIGHT;
	regs.r[6] = (6 << 27) | ((180/UNITS) << 14) | ((180/UNITS) << 1) | 1;
   _kernel_swi(OS_SpriteOp, &regs, &regs);

  clear_screen(buffer+HEADER);

  /* Bind the buffer to the context and make it current */
  if (OSMesaMakeCurrent( ctx, buffer+HEADER, GL_UNSIGNED_BYTE, WIDTH, HEIGHT )==GL_FALSE)
  {
  	exit(0);
  }

   /* Y coordinates increase downward on RISC OS */
   OSMesaPixelStore( OSMESA_Y_UP, 0 );

/* Initialise task */
	regs.r[0] = (unsigned int)310;
	regs.r[1] = (unsigned int)0x4b534154;
	regs.r[2] = (unsigned int)&task_name;
	regs.r[3] = 0;
   _kernel_swi(Wimp_Initialise, &regs, &regs);
	task_handle = regs.r[1];

/* Create window */
	regs.r[1] = (unsigned int)(&window_block) + 4;
   _kernel_swi(Wimp_CreateWindow, &regs, &regs);
   	window_block[0] = regs.r[0];

/* Create icon */
        icon_block[0] = window_block[0];
        icon_block[6] = (unsigned int)(buffer) + 20;
	icon_block[7] = (unsigned int)buffer;
	icon_block[8] = 7;
        regs.r[0] = 0;
        regs.r[1] = (unsigned int)&icon_block;
   _kernel_swi(Wimp_CreateIcon, &regs, &regs);

/* Open window */
        regs.r[1] = (unsigned int)&window_block;
   _kernel_swi(Wimp_OpenWindow, &regs, &regs);

}


void end( void )
{
/* Close window */
        regs.r[1] = (unsigned int)&window_block;
   _kernel_swi(Wimp_CloseWindow, &regs, &regs);

/* Delete window */
        regs.r[1] = (unsigned int)&window_block;
   _kernel_swi(Wimp_DeleteWindow, &regs, &regs);

/* Closedown */
	regs.r[0] = task_handle;
	regs.r[1] = (unsigned int)0x4b534154;
   _kernel_swi(Wimp_CloseDown, &regs, &regs);

   free(buffer);

   /* destroy the context */
   OSMesaDestroyContext( ctx );
}


int main( int argc, char *argv[] )
{
   int	quit = 0;

   setup();
   init();

do
{
	unsigned int	code;

	/* Poll */
		regs.r[0] = (unsigned int)0;
		regs.r[1] = (unsigned int)&message;
		regs.r[2] = 10;
	   _kernel_swi(Wimp_PollIdle, &regs, &regs);

	code = (unsigned int)regs.r[0];

/*
	fprintf(output, "code = %i\n", code);
*/

		switch (code)
		{
			case 0:		/* Null */
					draw();
					view_roty += 2.0;
					if (view_roty >= 360.0) view_roty = view_roty - 360.0;
					regs.r[0] = window_block[0];
					regs.r[1] = 0;
					regs.r[2] = -HEIGHT*UNITS;
					regs.r[3] = WIDTH*UNITS;
					regs.r[4] = 0;
					_kernel_swi(Wimp_ForceRedraw, &regs, &regs);
					break;
			case 2:		/* Open window */
					_kernel_swi(Wimp_OpenWindow, &regs, &regs);
					break;
                	case 3:		/* Close window */
                			_kernel_swi(Wimp_CloseWindow, &regs, &regs);
                			quit = 1;
                			break;
			case 6:		/* Button */
					switch (message[2])
					{
						case 2:
							/* Open menu */

							/* Get pointer position */
							regs.r[1] = (unsigned int)&workspace;
							_kernel_swi(Wimp_GetPointerInfo, &regs, &regs);
							regs.r[1] = (unsigned int)&menu_block;
							regs.r[2] = workspace[0] - 64;
							regs.r[3] = workspace[1];
							_kernel_swi(Wimp_CreateMenu, &regs, &regs);
							break;
						default:
							break;
					}
                                        break;
			case 9:		/* Menu selection */
					switch (message[0])
					{
						case 0:
							/* Lighting */
							menu_block.items[0].item_flags ^= 1;
							deinit();
							init();
							break;
						case 1:
							/* Quit */
							quit = 1;
							break;
						default:
							break;
					}

					/* Get pointer position */
					regs.r[1] = (unsigned int)&message;
					_kernel_swi(Wimp_GetPointerInfo, &regs, &regs);
					if (message[2] == 1)
					{
						regs.r[1] = (unsigned int)&menu_block;
						regs.r[2] = message[0] - 64;
						regs.r[3] = message[1];
						_kernel_swi(Wimp_CreateMenu, &regs, &regs);
					}
					break;
                	case 17:
                	case 18:
                			if (message[4] == 0)
                			{
                				quit = 1;
                                        }
                                        break;
                        default:
                          break;
		};
} while (quit == 0);

   deinit();
   end();
   return 0;
}
