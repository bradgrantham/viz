#include <cstdio>
#include <cstdlib>
#include <limits>
#include <algorithm>
#include <unistd.h>
#include <GLFW/glfw3.h>

#include "vectormath.h"

/*
move box to geometry.h/cpp
make Transform a class or c++ ize
    transform.h
set up draw code
*/

void print_matrix(float *v)
{
    printf("%f %f %f %f\n", v[0], v[1], v[2], v[3]);
    printf("%f %f %f %f\n", v[4], v[5], v[6], v[7]);
    printf("%f %f %f %f\n", v[8], v[9], v[10], v[11]);
    printf("%f %f %f %f\n", v[12], v[13], v[14], v[15]);
}

//------------------------------------------------------------------------
// geometry
//
// make box class, void clear(), bool empty(), vec3f extent()
//

void box_set_empty(vec3f& boxmin, vec3f& boxmax)
{
    boxmin.set(
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max());
    boxmax.set(
        -std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max());
}

void box_extend(vec3f& boxmin, vec3f& boxmax, float x, float y, float z)
{
    boxmin[0] = std::min(boxmin[0], x);
    boxmin[1] = std::min(boxmin[1], y);
    boxmin[2] = std::min(boxmin[2], z);
    boxmax[0] = std::max(boxmax[0], x);
    boxmax[1] = std::max(boxmax[1], y);
    boxmax[2] = std::max(boxmax[2], z);
}

void box_extend(vec3f& boxmin, vec3f& boxmax, float x, float y, float z, float r)
{
    boxmin[0] = std::min(boxmin[0], x - r);
    boxmin[1] = std::min(boxmin[1], y - r);
    boxmin[2] = std::min(boxmin[2], z - r);
    boxmax[0] = std::max(boxmax[0], x + r);
    boxmax[1] = std::max(boxmax[1], y + r);
    boxmax[2] = std::max(boxmax[2], z + r);
}

//------------------------------------------------------------------------
// Manipulator 
//
// drawing code...?
// make a class


enum xformMode {
    XFORM_MODE_ROTATE,	/* rotate in direction of mouse motion */
    XFORM_MODE_ROLL,		/* rotate around Z axis */
    XFORM_MODE_SCROLL,		/* translate in X-Y */
    XFORM_MODE_DOLLY		/* translate in Z */
};

typedef struct { 
    enum xformMode mode;
    mat4f matrix;

    mat4f frame;		/* The coordinate frame for this transform */

    vec3f worldX;		/* world X axis in this coordinate space */
    vec3f worldY;		/* world Y axis in this coordinate space */	
    vec3f worldZ;		/* world Z axis in this coordinate space */

    float referenceSize;	/* used to calculate translations */
    float motionScale;		/* for dynamic scaling, etc */

    rot4f rotation;		/* radians, x, y, z, like OpenGL */
    vec3f translation;
    vec3f scale;		/* scaled around center. */
    vec3f center;		/* ignore by setting to <0,0,0> */
} Transform;

/* Win32 math.h doesn't define M_PI. */
#if defined(_WIN32)
#if !defined(M_PI)
#define M_PI 3.14159265
#endif /* !defined(M_PI) */
#endif /* defined(WIN32) */

/* internal use */

static void dragToRotation(float dx, float dy, rot4f *rotation)
{
    float dist;

    /* XXX grantham 990825 - this "dist" doesn't make me confident. */
    /* but I put in the *10000 to decrease chance of underflow  (???) */
    dist = sqrt(dx * 10000 * dx * 10000 + dy * 10000 * dy * 10000) / 10000;
    /* dist = sqrt(dx * dx + dy * dy); */

    rotation->set(M_PI * dist, dy / dist, dx / dist, 0.0f);
}

static void calcViewMatrix(rot4f viewRotation, vec3f viewOffset,
    vec3f objCenter, vec3f objScale, mat4f *viewMatrix)
{
    mat4f tmp;

    /* These could be generated with OpenGL matrix functions */
    *viewMatrix = mat4f::identity;

    printf("viewOffset: %f %f %f\n",
        viewOffset[0],
        viewOffset[1],
        viewOffset[2]);
    *viewMatrix = mat4f::translation(viewOffset[0], viewOffset[1], viewOffset[2]);
    printf("translation: ");
    print_matrix(viewMatrix->m_v);

    *viewMatrix = *viewMatrix * mat4f(viewRotation);

    *viewMatrix = *viewMatrix * mat4f::scale(objScale[0], objScale[1], objScale[2]);

    *viewMatrix = *viewMatrix * mat4f::translation(-objCenter[0], -objCenter[1], -objCenter[2]);
}

/* external API */

void xformCalcMatrix(Transform *xform)
{
    calcViewMatrix(xform->rotation, xform->translation, xform->center,
        xform->scale, &xform->matrix);
}

void xformSetFrame(Transform *xform, const mat4f& frame)
{
    mat4f i;
    vec3f origin;

    xform->frame = frame;

    i.invert(xform->frame);
    origin = vec3f(0.0f, 0.0f, 0.0f) * i;
    xform->worldX = vec3f(1.0f, 0.0f, 0.0f) * i - origin;
    xform->worldY = vec3f(0.0f, 1.0f, 0.0f) * i - origin;
    xform->worldZ = vec3f(0.0f, 0.0f, 1.0f) * i - origin;
}

void xformMotion(Transform *xform, float dx, float dy)
{
    switch(xform->mode) {

	case XFORM_MODE_ROTATE:
	    if(dx != 0 || dy != 0) {
		rot4f localRotation;
		rot4f worldRotation;
		dragToRotation(dx, dy, &worldRotation);
		localRotation[0] = worldRotation[0];
		vec3f axis = xform->worldX * worldRotation[1] + 
                    xform->worldY * worldRotation[2];
		localRotation.set_axis(axis);
		xform->rotation = xform->rotation * localRotation;
	    }
	    break;

	case XFORM_MODE_ROLL:
            xform->rotation = xform->rotation * rot4f(M_PI * 2 * -dy, 0, 0, 1);
	    break;

	case XFORM_MODE_SCROLL:
	    xform->translation = xform->translation +
                xform->worldX * dx * xform->referenceSize * xform->motionScale +
                xform->worldY * dy * xform->referenceSize * xform->motionScale;
	    break;

	case XFORM_MODE_DOLLY:
	    xform->translation = xform->translation + xform->worldZ * dy * xform->referenceSize * xform->motionScale;
	    break;
    }

    calcViewMatrix(xform->rotation, xform->translation, xform->center,
        xform->scale, &xform->matrix);
}

void xformInitialize(Transform *xform)
{
    xform->worldX = vec3f(1.0f, 0.0f, 0.0f);
    xform->worldX = vec3f(0.0f, 1.0f, 0.0f);
    xform->worldX = vec3f(0.0f, 0.0f, 1.0f);

    xform->center.set(0, 0, 0);

    xform->scale.set(1, 1, 1);

    /* diagonal of 2-high cube */
    xform->referenceSize = 3.465;

    xform->motionScale = 1.0;

    xform->translation.set(0, 0, 0);

    xform->rotation.set(0, 1, 0, 0);

    calcViewMatrix(xform->rotation, xform->translation, xform->center,
        xform->scale, &xform->matrix);

    xform->mode = XFORM_MODE_ROTATE;
}


void xformInitializeViewFromBox(Transform *xform, const vec3f& boxmin, const vec3f& boxmax, float fov)
{
    xform->worldX = vec3f(1.0f, 0.0f, 0.0f);
    xform->worldX = vec3f(0.0f, 1.0f, 0.0f);
    xform->worldX = vec3f(0.0f, 0.0f, 1.0f);

    xform->center = vec3f(0.0f, 0.0f, 0.0f);

    xform->scale = vec3f(1.0f, 1.0f, 1.0f);

    if(boxmax[0] - boxmin[0] > boxmax[1] - boxmin[1] &&
	boxmax[0] - boxmin[0] > boxmax[2] - boxmin[2])
        xform->referenceSize = boxmax[0] - boxmin[0];
    else if(boxmax[1] - boxmin[1] > boxmax[2] - boxmin[2])
        xform->referenceSize = boxmax[1] - boxmin[1];
    else
        xform->referenceSize = boxmax[2] - boxmin[2];

    xform->motionScale = 1.0;

    xform->translation.set(0, 0, -xform->referenceSize / cosf(fov / 2.0));

    xform->rotation.set(0, 1, 0, 0);

    calcViewMatrix(xform->rotation, xform->translation, xform->center,
        xform->scale, &xform->matrix);

    xform->mode = XFORM_MODE_ROTATE;
}

//------------------------------------------------------------------------

static Transform gSceneTransform;
static Transform gObjectTransform;
static Transform *gCurrentTransform = NULL;

static bool gStreamFrames = false;

static int gWindowWidth;
static int gWindowHeight;

static double gOldMouseX, gOldMouseY;
static int gButtonPressed = -1;

//------------------------------------------------------------------------

static float object_ambient[4] = {.1, .1, .1, 1};
static float object_diffuse[4] = {.8, .8, .8, 1};
static float object_specular[4] = {.5, .5, .5, 1};
static float object_shininess = 50;


//----------------------------------------------------------------------------
// Actual GL functions

#define CHECK_OPENGL(l) {int _glerr ; if((_glerr = glGetError()) != GL_NO_ERROR) printf("GL Error: %04X at %d\n", _glerr, l); }

struct vertex
{
    float v[3];
    float n[3];
};

const struct vertex vertices[] = {
    {{73.840424, -40.755272, 106.148109}, {0.519144, 0.442333, 0.731321}},
    {{-19.752251, 103.538597, 85.178741}, {0.519144, 0.442333, 0.731321}},
    {{-22.195251, 5.656128, 139.905396}, {0.519144, 0.442333, 0.731321}},
    {{73.840424, -40.755272, 106.148109}, {0.519144, 0.442333, 0.731321}},
    {{98.520752, 106.222244, 11.064491}, {0.519144, 0.442333, 0.731321}},
    {{-19.752251, 103.538597, 85.178741}, {0.519144, 0.442333, 0.731321}},
    {{73.840424, -40.755272, 106.148109}, {0.519144, 0.442333, 0.731321}},
    {{135.184387, 49.762146, 19.243591}, {0.519144, 0.442333, 0.731321}},
    {{98.520752, 106.222244, 11.064491}, {0.519144, 0.442333, 0.731321}},
    {{-25.951000, -142.532745, 1.932705}, {-0.061883, -0.722689, 0.688397}},
    {{-22.195099, 5.656044, 139.905396}, {-0.061883, -0.722689, 0.688397}},
    {{-96.937134, -51.414917, 79.640381}, {-0.061883, -0.722689, 0.688397}},
    {{-25.951000, -142.532745, 1.932705}, {-0.061883, -0.722689, 0.688397}},
    {{73.840385, -40.755268, 106.148178}, {-0.061883, -0.722689, 0.688397}},
    {{-22.195099, 5.656044, 139.905396}, {-0.061883, -0.722689, 0.688397}},
    {{-25.951000, -142.532745, 1.932705}, {-0.061883, -0.722689, 0.688397}},
    {{41.326958, -138.328751, 12.379768}, {-0.061883, -0.722689, 0.688397}},
    {{73.840385, -40.755268, 106.148178}, {-0.061883, -0.722689, 0.688397}},
    {{-139.567596, 32.339836, -23.409714}, {-0.826855, -0.552100, -0.107221}},
    {{-25.950974, -142.532730, 1.932693}, {-0.826855, -0.552100, -0.107221}},
    {{-96.937088, -51.414902, 79.640388}, {-0.826855, -0.552100, -0.107221}},
    {{-139.567596, 32.339836, -23.409714}, {-0.826855, -0.552100, -0.107221}},
    {{-68.397202, -59.061600, -101.076385}, {-0.826855, -0.552100, -0.107221}},
    {{-25.950974, -142.532730, 1.932693}, {-0.826855, -0.552100, -0.107221}},
    {{-19.752129, 103.538391, 85.178711}, {-0.765892, 0.361717, 0.531573}},
    {{-96.937096, -51.414951, 79.640350}, {-0.765892, 0.361717, 0.531573}},
    {{-22.195099, 5.656006, 139.905334}, {-0.765892, 0.361717, 0.531573}},
    {{-19.752129, 103.538391, 85.178711}, {-0.765892, 0.361717, 0.531573}},
    {{-139.567596, 32.339756, -23.409744}, {-0.765892, 0.361717, 0.531573}},
    {{-96.937096, -51.414951, 79.640350}, {-0.765892, 0.361717, 0.531573}},
    {{-19.752129, 103.538391, 85.178711}, {-0.765892, 0.361717, 0.531573}},
    {{-109.200485, 93.235596, -21.190659}, {-0.765892, 0.361717, 0.531573}},
    {{-139.567596, 32.339756, -23.409744}, {-0.765892, 0.361717, 0.531573}},
    {{98.520813, 106.222237, 11.064453}, {-0.054479, 0.997222, -0.050799}},
    {{-109.200531, 93.235657, -21.190613}, {-0.054479, 0.997222, -0.050799}},
    {{-19.752136, 103.538589, 85.178711}, {-0.054479, 0.997222, -0.050799}},
    {{98.520813, 106.222237, 11.064453}, {-0.054479, 0.997222, -0.050799}},
    {{8.758667, 95.899689, -95.353668}, {-0.054479, 0.997222, -0.050799}},
    {{-109.200531, 93.235657, -21.190613}, {-0.054479, 0.997222, -0.050799}},
    {{135.184418, 49.762268, 19.243624}, {0.724657, 0.387270, -0.569995}},
    {{8.758682, 95.899727, -95.353622}, {0.724657, 0.387270, -0.569995}},
    {{98.520859, 106.222275, 11.064529}, {0.724657, 0.387270, -0.569995}},
    {{135.184418, 49.762268, 19.243624}, {0.724657, 0.387270, -0.569995}},
    {{21.995865, -6.184021, -139.914566}, {0.724657, 0.387270, -0.569995}},
    {{8.758682, 95.899727, -95.353622}, {0.724657, 0.387270, -0.569995}},
    {{135.184418, 49.762268, 19.243624}, {0.724657, 0.387270, -0.569995}},
    {{102.376770, -48.400955, -74.546539}, {0.724657, 0.387270, -0.569995}},
    {{21.995865, -6.184021, -139.914566}, {0.724657, 0.387270, -0.569995}},
    {{73.840393, -40.755310, 106.148071}, {0.881459, -0.445034, 0.158036}},
    {{102.376785, -48.400986, -74.546463}, {0.881459, -0.445034, 0.158036}},
    {{135.184372, 49.762054, 19.243523}, {0.881459, -0.445034, 0.158036}},
    {{73.840393, -40.755310, 106.148071}, {0.881459, -0.445034, 0.158036}},
    {{41.327011, -138.328705, 12.379734}, {0.881459, -0.445034, 0.158036}},
    {{102.376785, -48.400986, -74.546463}, {0.881459, -0.445034, 0.158036}},
    {{102.376785, -48.400909, -74.546478}, {0.143487, -0.777714, -0.612023}},
    {{-68.397263, -59.061600, -101.076355}, {0.143487, -0.777714, -0.612023}},
    {{21.995842, -6.183952, -139.914551}, {0.143487, -0.777714, -0.612023}},
    {{102.376785, -48.400909, -74.546478}, {0.143487, -0.777714, -0.612023}},
    {{-25.951050, -142.532700, 1.932678}, {0.143487, -0.777714, -0.612023}},
    {{-68.397263, -59.061600, -101.076355}, {0.143487, -0.777714, -0.612023}},
    {{102.376785, -48.400909, -74.546478}, {0.143487, -0.777714, -0.612023}},
    {{41.327003, -138.328690, 12.379752}, {0.143487, -0.777714, -0.612023}},
    {{-25.951050, -142.532700, 1.932678}, {0.143487, -0.777714, -0.612023}},
    {{8.758606, 95.899704, -95.353638}, {-0.560447, 0.306673, -0.769318}},
    {{-139.567703, 32.339783, -23.409790}, {-0.560447, 0.306673, -0.769318}},
    {{-109.200600, 93.235672, -21.190693}, {-0.560447, 0.306673, -0.769318}},
    {{8.758606, 95.899704, -95.353638}, {-0.560447, 0.306673, -0.769318}},
    {{-68.397316, -59.061653, -101.076370}, {-0.560447, 0.306673, -0.769318}},
    {{-139.567703, 32.339783, -23.409790}, {-0.560447, 0.306673, -0.769318}},
    {{8.758606, 95.899704, -95.353638}, {-0.560447, 0.306673, -0.769318}},
    {{21.995825, -6.184006, -139.914551}, {-0.560447, 0.306673, -0.769318}},
    {{-68.397316, -59.061653, -101.076370}, {-0.560447, 0.306673, -0.769318}},

};

float center[3] = {2.379061, 0.722852, 0.344983};
float size = 139.909981;

int triangle_count = 24;

void draw_object(float object_time, bool draw_wireframe, bool flat_shade)
{
    CHECK_OPENGL(__LINE__);
    if(flat_shade)
        glShadeModel(GL_FLAT);
    else
        glShadeModel(GL_SMOOTH);

    // glPushMatrix();

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, object_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, object_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, object_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, object_shininess);

    // float object_angle_x = object_time * 45;
    // float object_angle_y = object_time * 45 / 1.3;

    // glRotatef(object_angle_x, 1, 0, 0);
    // glRotatef(object_angle_y, 0, 1, 0);
    // glScalef(1.0 / size, 1.0 / size, 1.0 / size);
    // glTranslatef(-center[0], -center[1], -center[2]);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(vertices[0]), (void*)&vertices[0].v[0]); 
    glNormalPointer(GL_FLOAT, sizeof(vertices[0]), (void*)&vertices[0].n[0]);

    if(draw_wireframe) {
        for(int i = 0; i < triangle_count; i++)
            glDrawArrays(GL_LINE_LOOP, i * 3, 3);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, triangle_count * 3);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    // glPopMatrix();
    CHECK_OPENGL(__LINE__);
}

void initialize_gl()
{
    glClearColor(1, 0, 0, 0);
    CHECK_OPENGL(__LINE__);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -3);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-.1, .1, -.1, .1, .2, 100);

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_LIGHT0);
    // glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_color);
    // glLightfv(GL_LIGHT0, GL_SPECULAR, light0_color);

    // glEnable(GL_LIGHT1);
    // glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_color);
    // glLightfv(GL_LIGHT1, GL_SPECULAR, light1_color);

    glEnable(GL_LIGHTING);
    CHECK_OPENGL(__LINE__);

    glEnable(GL_CULL_FACE);

    glEnable(GL_NORMALIZE);

    CHECK_OPENGL(__LINE__);

    vec3f boxmin, boxmax;
    box_set_empty(boxmin, boxmax);
    for(int i = 0; i < triangle_count * 3; i++) {
        box_extend(boxmin, boxmax, vertices[i].v[0], vertices[i].v[1], vertices[i].v[2]);
    }
    xformInitializeViewFromBox(&gSceneTransform, boxmin, boxmax, .57595f);

    xformInitialize(&gObjectTransform);
    gObjectTransform.scale[0] = .4;
    gObjectTransform.scale[1] = .4;
    gObjectTransform.scale[2] = .4;
    xformCalcMatrix(&gObjectTransform);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW: %s\n", description);
}

static void key(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS) {
        switch(key) {
            case 'R':
                gCurrentTransform->mode = XFORM_MODE_ROTATE;
                break;

            case 'O':
                gCurrentTransform->mode = XFORM_MODE_ROLL;
                break;

            case 'X':
                gCurrentTransform->mode = XFORM_MODE_SCROLL;
                break;

            case 'Z':
                gCurrentTransform->mode = XFORM_MODE_DOLLY;
                break;

            case '1':
                gCurrentTransform = &gSceneTransform;
                break;

            case '2':
                gCurrentTransform = &gObjectTransform;
                break;

            case 'Q': case '\033':
                printf("q\n");
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
        }
    }
}

static void resize(GLFWwindow *window, int x, int y)
{
    glfwGetFramebufferSize(window, &gWindowWidth, &gWindowHeight);
    glViewport(0, 0, gWindowWidth, gWindowWidth);
}

static void button(GLFWwindow *window, int b, int action, int mods)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    if(b == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        gButtonPressed = 1;
        printf("button down, %f, %f\n", x, y);
#if defined(DEBUG)
	printf("    now %f: %d %d\n", timerGet(motionTimer), x - gOldMouseX, y - gOldMouseY);
#endif /* defined(DEBUG) */
    } else {
        gButtonPressed = -1;
        printf("button up, %f, %f\n", x, y);
	gOldMouseX = x;
	gOldMouseY = y;
    }
}

static void motion(GLFWwindow *window, double x, double y)
{
    double dx, dy;

    dx = x - gOldMouseX;
    dy = y - gOldMouseY;

    gOldMouseX = x;
    gOldMouseY = y;


    if(gButtonPressed == 1) {
        printf("drag %f, %f\n", dx, dy);

        xformMotion(gCurrentTransform, dx / 512.0, dy / 512.0);
        if(gCurrentTransform == &gSceneTransform)
            xformSetFrame(&gObjectTransform, gSceneTransform.matrix);
    }
}

static void scroll(GLFWwindow *window, double dx, double dy)
{
    printf("scroll %f, %f\n", dx, dy);
}

float frustLeft		= -0.66f;
float frustRight	= 0.66f;
float frustBottom	= -0.66f;
float frustTop		= 0.66f;

static void redraw(GLFWwindow *window)
{
    float nearClip, farClip;
    CHECK_OPENGL(__LINE__);

    frustLeft = frustBottom * gWindowWidth / gWindowHeight;
    frustRight = frustTop * gWindowWidth / gWindowHeight;

    /* XXX - need to create new box from all subordinate boxes */
    nearClip = - gSceneTransform.translation[2] - gSceneTransform.referenceSize;
    farClip = - gSceneTransform.translation[2] + gSceneTransform.referenceSize;
    if(nearClip < 0.1 * gSceneTransform.referenceSize)
	nearClip = 0.1 * gSceneTransform.referenceSize;
    if(farClip < 0.2 * gSceneTransform.referenceSize)
	nearClip = 0.2 * gSceneTransform.referenceSize;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(frustLeft * nearClip, frustRight * nearClip, frustBottom * nearClip, frustTop * nearClip, nearClip, farClip);
    CHECK_OPENGL(__LINE__);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    CHECK_OPENGL(__LINE__);

    glPushMatrix();
    glMultMatrixf((float *)gSceneTransform.matrix.m_v);
    print_matrix(gSceneTransform.matrix.m_v);
    CHECK_OPENGL(__LINE__);

    /* draw floor, draw shadow, etc */
    CHECK_OPENGL(__LINE__);

    glPushMatrix();
    glMultMatrixf((float *)gObjectTransform.matrix.m_v);
    draw_object(0, false, false);

    glPopMatrix();

    glPopMatrix();

}

int main()
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if(!glfwInit())
        exit(EXIT_FAILURE);

    window = glfwCreateWindow(gWindowWidth = 512, gWindowHeight = 512, "Spin", NULL, NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stdout, "Couldn't open main window\n");
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    gCurrentTransform = &gObjectTransform;
    initialize_gl();

    glfwSetKeyCallback(window, key);
    glfwSetMouseButtonCallback(window, button);
    glfwSetCursorPosCallback(window, motion);
    glfwSetScrollCallback(window, scroll);
    glfwSetFramebufferSizeCallback(window, resize);
    glfwSetWindowRefreshCallback(window, redraw);

    while (!glfwWindowShouldClose(window)) {

        redraw(window);

        glfwSwapBuffers(window);

        if(gStreamFrames)
            glfwPollEvents();
        else
            glfwWaitEvents();
    }

    glfwTerminate();
}
