#include <iostream>
#include <cmath>
#include <cstring>
#include "raylib/raylib.hpp"

//typedef void (*ShapeDrawFunction)(Particle&);

//static gy : f32 = 0.0002;
//static gx : f32 = 0.0002;
constexpr double gy = 0.001;
constexpr double gx = 0.001;

//const N_PARTICLES : usize = 8192/*  * 35*/;
constexpr ushort N_PARTICLES = 7;
constexpr double G = 0.00001;

struct Particle {
    double            x;
    double            y;
    double            speed_x;
    double            speed_y;
    double            mass;
    float             size;
    void              (*shapeDrawingFunction)(Particle&);
    raylib::Color     color;
};

static Particle particles[N_PARTICLES];

static double SCALE    = 0.25;
static ushort SPEED    = /*40*/0;
static short  X_CENTER = 0;
static short  Y_CENTER = 0;

void dotDrawingFunction       (Particle& particle) {
    particle.color.DrawPixel(X_CENTER + particle.x*SCALE, Y_CENTER + particle.y*SCALE);
}
void rectangleDrawingFunction (Particle& particle) {
    particle.color.DrawRectangle(X_CENTER + particle.x*SCALE, Y_CENTER + particle.y*SCALE, particle.size*SCALE, particle.size*SCALE);
}
void ellipseDrawingFunction   (Particle& particle) {
    //DrawCircle();
    raylib::Vector2 circle(X_CENTER + particle.x*SCALE, Y_CENTER + particle.y*SCALE);
    circle.DrawCircle(particle.size*SCALE, particle.color);
}

// this update function treats all particles as stars -- their sizes and masses are related and they attract each other... but they not collide, after all, we live in a 3D world... don't we?
void updatePositionsStars() {
    //let len : usize =  particles.len();
    //let particles_ptr = particles.as_mut_ptr();

    for (ushort i=0; i<N_PARTICLES; i++) {

        Particle& particle1 = particles[i];

        for (ushort j=i+1; j<N_PARTICLES; j++) {

            Particle& particle2 = particles[j];

//            let (x_acceleration1, y_acceleration1, x_acceleration2, y_acceleration2) = get_accelerations_due_to_gravity(&particle1, &particle2);

            // the (quadratic) distances² involved between the two bodies
            double x_distance   = particle2.x - particle1.x;
            double x_distance_2 = pow(x_distance, 2);
            double y_distance   = particle2.y - particle1.y;
            double y_distance_2 = pow(y_distance, 2);
            double distance_2   = x_distance_2 + y_distance_2;
            // inelastic collision ?
            if (distance_2 <= ( pow(particle1.size, 2) + pow(particle2.size, 2) ) ) {
                //double sx = (particle1.sx*particle1.mass + particle2.sx*particle2.mass) / (particle1.mass+particle2.mass);
                //double sy = (particle1.sy*particle1.mass + particle2.sy*particle2.mass) / (particle1.mass+particle2.mass);
                particle2.speed_x = particle1.speed_x = (particle1.speed_x*particle1.mass + particle2.speed_x*particle2.mass) / (particle1.mass+particle2.mass);
                particle2.speed_y = particle1.speed_y = (particle1.speed_y*particle1.mass + particle2.speed_y*particle2.mass) / (particle1.mass+particle2.mass);
                continue;
            }
            // force due to gravity
            double force = G * ((particle1.mass * particle2.mass) / distance_2);
            // vectors
            double sin = sqrt(y_distance_2 / distance_2);
            double cos = sqrt(x_distance_2 / distance_2);
            // accelerations involved
            double acceleration1   = force / particle1.mass;
            double acceleration2   = force / particle2.mass;
            double x_acceleration1 = acceleration1 * cos * (x_distance >= 0.0 ? 1.0 : -1.0);
            double y_acceleration1 = acceleration1 * sin * (y_distance >= 0.0 ? 1.0 : -1.0);
            double x_acceleration2 = acceleration2 * cos * (x_distance <= 0.0 ? 1.0 : -1.0);
            double y_acceleration2 = acceleration2 * sin * (y_distance <= 0.0 ? 1.0 : -1.0);
            // compute accelerations into inertia
            particle1.speed_x += x_acceleration1;
            particle1.speed_y += y_acceleration1;
            particle2.speed_x += x_acceleration2;
            particle2.speed_y += y_acceleration2;
        }

        // after computing all accelerations for particle1, we may update its position
        particle1.x += particle1.speed_x;
        particle1.y += particle1.speed_y;

    }
}


void guiLoop(raylib::Window w) {

    ushort showMsgCount  = 600;
    char msg[5][1024]    = {"Welcome to DomBalls. Press 'h' at any time for help.", "", "", "", ""};
    bool showFPS         = true;
    bool keepOrbitsTrack = false;

    raylib::Color background(BLACK);
    raylib::Color textColor(LIGHTGRAY);

    // Main game loop
    while (!w.ShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_EQUAL)) {
            SPEED += 20;
            showMsgCount = 120;
            sprintf(msg[0], "SPEED: %d", SPEED);
            msg[1][0] = '\0';
        } else if (IsKeyDown(KEY_MINUS)) {
            SPEED -= 20;
            showMsgCount = 120;
            sprintf(msg[0], "SPEED: %d", SPEED);
            msg[1][0] = '\0';
        } else if (IsKeyDown(KEY_H)) {
            showMsgCount = 120;
            strcpy(msg[0], "DomBalls' (h)elp. Press:");
            strcpy(msg[1], " (a), (z):                      zoom in and out");
            strcpy(msg[2], " (left), (right), (up), (down): move the spyglass");
            strcpy(msg[3], " (=), (-)                     : zoom in and out");
            strcpy(msg[4], " (f)                          : toggle FPS");
        } else if (IsKeyPressed(KEY_F)) {
            showFPS = !showFPS;
        } else if (IsKeyPressed(KEY_R)) {
            keepOrbitsTrack = !keepOrbitsTrack;
        } else if (IsKeyDown(KEY_A)) {
            SCALE += 0.001;
            showMsgCount = 120;
            sprintf(msg[0], "SCALE: %lf", SCALE);
            msg[1][0] = '\0';
        } else if (IsKeyDown(KEY_Z)) {
            SCALE -= 0.001;
            showMsgCount = 120;
            sprintf(msg[0], "SCALE: %lf", SCALE);
            msg[1][0] = '\0';
        }

        if (IsKeyDown(KEY_LEFT)) {
            X_CENTER += 2;
        } else if (IsKeyDown(KEY_RIGHT)) {
            X_CENTER -= 2;
        } else if (IsKeyDown(KEY_UP)) {
            Y_CENTER += 2;
        } else if (IsKeyDown(KEY_DOWN)) {
            Y_CENTER -= 2;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        if (!keepOrbitsTrack) {
            background.ClearBackground();
        }

        for (ushort i=0; i<N_PARTICLES; i++) {
            particles[i].shapeDrawingFunction(particles[i]);
        }

        if (showFPS) {
            DrawFPS(10, 10);
        }

        if (showMsgCount > 0) {
            for (ushort i=0; i<5; i++) {
                if (strlen(msg[i]) > 0) {
                    textColor.DrawText(msg[i], 10, 30+(i*20), 20);
                } else {
                    break;
                }
            }
            showMsgCount--;
        }


        EndDrawing();

        for (ushort i=0; i<SPEED; i++) {
            updatePositionsStars();
        }

        //----------------------------------------------------------------------------------
    }
}

int main()
{
	// Initialization
	//--------------------------------------------------------------------------------------
	int screenWidth = GetScreenWidth();
	int screenHeight = GetScreenHeight();
    screenWidth  = screenWidth  > 0 ? screenWidth  : 1920;
    screenHeight = screenHeight > 0 ? screenHeight : 1080;
	raylib::Window w(screenWidth, screenHeight, "DomBalls");

	if (!w.IsFullscreen()) {
        w.ToggleFullscreen();
        screenWidth  = 1366;
        screenHeight = 768;
    }

    double center_x = (double)screenWidth  / (2.0 * SCALE);
    double center_y = (double)screenHeight / (2.0 * SCALE);

    X_CENTER = 0;
    Y_CENTER = 0;

	SetTargetFPS(60);
	//--------------------------------------------------------------------------------------

    ushort i  = 0;
// interesting things for constexpr double G = 0.0000001;
/*    // unstable mars kicks earth far from the sun, catapults our moon to the asteroid belt and collides with venus, causing a reasonably elliptical mercury orbit on the process. Some ~1000 years ahead, moon collides with earth
    particles[i].x=1366.0/2.0; particles[i].y=680.0;     particles[i].size=5;   particles[i].mass=10.0;   particles[i].speed_x=0.0008;      particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BLUE);       i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=768.0/2.0; particles[i].size=28;  particles[i].mass=2000.0; particles[i].speed_x=-0.00000546; particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(YELLOW);     i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=560.0;     particles[i].size=6;   particles[i].mass=11.0;   particles[i].speed_x=0.001;       particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(ORANGE);     i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=694.5;     particles[i].size=1;   particles[i].mass=1.0;    particles[i].speed_x=0.001030;    particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=dotDrawingFunction;     particles[i].color=raylib::Color(WHITE);      i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=334.0;     particles[i].size=3;   particles[i].mass=3.0;    particles[i].speed_x=-0.0022;     particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(DARKPURPLE); i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=2.0;       particles[i].size=3;   particles[i].mass=3.8;    particles[i].speed_x=-0.000729;   particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(RED);        i+=1;
    // earth gets to mars orbit, which collides with the moon and then with venus, leading mercury to collide with the sun
    particles[i].x=1366.0/2.0; particles[i].y=680.0;     particles[i].size=5;   particles[i].mass=10.0;   particles[i].speed_x=0.0008;      particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BLUE);       i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=768.0/2.0; particles[i].size=28;  particles[i].mass=2000.0; particles[i].speed_x=-0.00000546; particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(YELLOW);     i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=560.0;     particles[i].size=6;   particles[i].mass=11.0;   particles[i].speed_x=0.001;       particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(ORANGE);     i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=694.5;     particles[i].size=1;   particles[i].mass=1.0;    particles[i].speed_x=0.001030;    particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=dotDrawingFunction;     particles[i].color=raylib::Color(WHITE);      i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=334.0;     particles[i].size=3;   particles[i].mass=3.0;    particles[i].speed_x=-0.0022;     particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(DARKPURPLE); i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=6.0;       particles[i].size=3;   particles[i].mass=3.8;    particles[i].speed_x=-0.000729;   particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(RED);        i+=1;
    // mars and earth swap orbits, then marks collides with venus
    particles[i].x=1366.0/2.0; particles[i].y=680.0;     particles[i].size=5;   particles[i].mass=10.0;   particles[i].speed_x=0.0008;      particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BLUE);       i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=768.0/2.0; particles[i].size=28;  particles[i].mass=2000.0; particles[i].speed_x=-0.00000546; particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(YELLOW);     i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=560.0;     particles[i].size=6;   particles[i].mass=11.0;   particles[i].speed_x=0.001;       particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(ORANGE);     i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=694.5;     particles[i].size=1;   particles[i].mass=1.0;    particles[i].speed_x=0.001030;    particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=dotDrawingFunction;     particles[i].color=raylib::Color(WHITE);      i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=334.0;     particles[i].size=3;   particles[i].mass=3.0;    particles[i].speed_x=-0.0022;     particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(DARKPURPLE); i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=766.0;     particles[i].size=3;   particles[i].mass=3.8;    particles[i].speed_x=0.000720;   particles[i].speed_y=0.0; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(RED);        i+=1;
        // reasonably stable orbit
    particles[i].x=center_x;       particles[i].y=center_y+296.0;  particles[i].size=5;   particles[i].mass=10.0;   particles[i].speed_x=0.0008;      particles[i].speed_y=0.0;         particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BLUE);       i+=1;
    particles[i].x=center_x;       particles[i].y=center_y;        particles[i].size=28;  particles[i].mass=2000.0; particles[i].speed_x=-6.715e-6;   particles[i].speed_y=-1.349e-06;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(YELLOW);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+176.0;  particles[i].size=6;   particles[i].mass=11.0;   particles[i].speed_x=0.001;       particles[i].speed_y=0.0;         particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(ORANGE);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+310.5;  particles[i].size=1;   particles[i].mass=1.0;    particles[i].speed_x=0.001030;    particles[i].speed_y=0.0;         particles[i].shapeDrawingFunction=dotDrawingFunction;     particles[i].color=raylib::Color(WHITE);      i+=1;
    particles[i].x=center_x;       particles[i].y=center_y-50.0;   particles[i].size=3;   particles[i].mass=3.0;    particles[i].speed_x=-0.0022;     particles[i].speed_y=0.0;         particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(DARKPURPLE); i+=1;
    particles[i].x=center_x-411.0; particles[i].y=center_y;        particles[i].size=3;   particles[i].mass=3.8;    particles[i].speed_x=0.0;         particles[i].speed_y=0.000710;    particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(RED);        i+=1;
    // slightly elliptical mars gets almost kicked out of the solar system by earth (with the help of jupiter), just to get back to interesting interactions until it finally collides with jupiter
    particles[i].x=center_x;       particles[i].y=center_y+296.0;  particles[i].size=5;   particles[i].mass=10.0;   particles[i].speed_x=0.0008;      particles[i].speed_y=2.0840476894275e-05;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BLUE);       i+=1;
    particles[i].x=center_x;       particles[i].y=center_y;        particles[i].size=28;  particles[i].mass=2000.0; particles[i].speed_x=-6.715e-6;   particles[i].speed_y=2.0840476894275e-05;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(YELLOW);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+176.0;  particles[i].size=6;   particles[i].mass=11.0;   particles[i].speed_x=0.001;       particles[i].speed_y=2.0840476894275e-05;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(ORANGE);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+310.3;  particles[i].size=1;   particles[i].mass=1.0;    particles[i].speed_x=0.001030;    particles[i].speed_y=2.0840476894275e-05;  particles[i].shapeDrawingFunction=dotDrawingFunction;     particles[i].color=raylib::Color(WHITE);      i+=1;
    particles[i].x=center_x;       particles[i].y=center_y-50.0;   particles[i].size=3;   particles[i].mass=3.0;    particles[i].speed_x=-0.0022;     particles[i].speed_y=2.0840476894275e-05;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(DARKPURPLE); i+=1;
    particles[i].x=center_x-411.0; particles[i].y=center_y;        particles[i].size=3;   particles[i].mass=3.8;    particles[i].speed_x=0.0;         particles[i].speed_y=0.00068915252310573;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(RED);        i+=1;
    particles[i].x=center_x+920.0; particles[i].y=center_y;        particles[i].size=15;  particles[i].mass=100.0;  particles[i].speed_x=0.0;         particles[i].speed_y=-0.00045;             particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BROWN);      i+=1;
    // mars makes the moon collides with earth, then gets in an orbit between earth and venus to be, later sling shooted out of the solar system by jupiter
    particles[i].x=center_x;       particles[i].y=center_y+296.0;  particles[i].size=5;   particles[i].mass=10.0;   particles[i].speed_x=0.0008;      particles[i].speed_y=2.0840476894275e-05;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BLUE);       i+=1;
    particles[i].x=center_x;       particles[i].y=center_y;        particles[i].size=28;  particles[i].mass=2000.0; particles[i].speed_x=-6.715e-6;   particles[i].speed_y=2.0840476894275e-05;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(YELLOW);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+176.0;  particles[i].size=6;   particles[i].mass=11.0;   particles[i].speed_x=0.001;       particles[i].speed_y=2.0840476894275e-05;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(ORANGE);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+310.5;  particles[i].size=1;   particles[i].mass=1.0;    particles[i].speed_x=0.001029;    particles[i].speed_y=2.0840476894275e-05;  particles[i].shapeDrawingFunction=dotDrawingFunction;     particles[i].color=raylib::Color(WHITE);      i+=1;
    particles[i].x=center_x;       particles[i].y=center_y-50.0;   particles[i].size=3;   particles[i].mass=3.0;    particles[i].speed_x=-0.0023;     particles[i].speed_y=2.0840476894275e-05;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(DARKPURPLE); i+=1;
    particles[i].x=center_x-451.0; particles[i].y=center_y;        particles[i].size=3;   particles[i].mass=3.8;    particles[i].speed_x=0.0;         particles[i].speed_y=0.00067915252310573;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(RED);        i+=1;
    particles[i].x=center_x+920.0; particles[i].y=center_y;        particles[i].size=15;  particles[i].mass=100.0;  particles[i].speed_x=0.0;         particles[i].speed_y=-0.00045;             particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BROWN);      i+=1;
    // 100 greater G
    // mars steals the moon, then collides with jupiter; slightly elliptical mercury, who later collided with venus, plays a hole in helping venus to set earth in an ice-age elliptical orbit before some nice interactions with jupiter, which was almost able to capture earth as his moon twice, kicked earth further out on a 300-years orbit
    particles[i].x=center_x;       particles[i].y=center_y+296.0;  particles[i].size=5;   particles[i].mass=10.0;   particles[i].speed_x=0.008;      particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BLUE);       i+=1;
    particles[i].x=center_x;       particles[i].y=center_y;        particles[i].size=28;  particles[i].mass=2000.0; particles[i].speed_x=-6.715e-5;  particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(YELLOW);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+176.0;  particles[i].size=6;   particles[i].mass=11.0;   particles[i].speed_x=0.01;       particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(ORANGE);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+310.5;  particles[i].size=1;   particles[i].mass=1.0;    particles[i].speed_x=0.01029;    particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=dotDrawingFunction;     particles[i].color=raylib::Color(WHITE);      i+=1;
    particles[i].x=center_x;       particles[i].y=center_y-50.0;   particles[i].size=3;   particles[i].mass=3.0;    particles[i].speed_x=-0.023;     particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(DARKPURPLE); i+=1;
    particles[i].x=center_x-451.0; particles[i].y=center_y;        particles[i].size=3;   particles[i].mass=3.8;    particles[i].speed_x=0.0;        particles[i].speed_y=0.0065915252310573;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(RED);        i+=1;
    particles[i].x=center_x+920.0; particles[i].y=center_y;        particles[i].size=15;  particles[i].mass=100.0;  particles[i].speed_x=0.0;        particles[i].speed_y=-0.0045;             particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BROWN);      i+=1;
    // a small change on the moon causes mars to be kicked out of the solar system -- all the rest is stable with the exception with mercury colliding with the sun in a ~million years
    particles[i].x=center_x;       particles[i].y=center_y+296.0;  particles[i].size=5;   particles[i].mass=10.0;   particles[i].speed_x=0.008;      particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BLUE);       i+=1;
    particles[i].x=center_x;       particles[i].y=center_y;        particles[i].size=28;  particles[i].mass=2000.0; particles[i].speed_x=-6.715e-5;  particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(YELLOW);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+176.0;  particles[i].size=6;   particles[i].mass=11.0;   particles[i].speed_x=0.01;       particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(ORANGE);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+281.5;  particles[i].size=1;   particles[i].mass=1.0;    particles[i].speed_x=+0.01029;   particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=dotDrawingFunction;     particles[i].color=raylib::Color(WHITE);      i+=1;
    particles[i].x=center_x;       particles[i].y=center_y-50.0;   particles[i].size=3;   particles[i].mass=3.0;    particles[i].speed_x=-0.023;     particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(DARKPURPLE); i+=1;
    particles[i].x=center_x-451.0; particles[i].y=center_y;        particles[i].size=3;   particles[i].mass=3.8;    particles[i].speed_x=0.0;        particles[i].speed_y=0.0065915252310573;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(RED);        i+=1;
    particles[i].x=center_x+920.0; particles[i].y=center_y;        particles[i].size=15;  particles[i].mass=100.0;  particles[i].speed_x=0.0;        particles[i].speed_y=-0.0045;             particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BROWN);      i+=1;
    // a slight change on moon orbit (no change in momentum) makes mars become a moon of jupiter for a while, then collide with mercury and, then, both colliding with venus while setting earth on a million-year stable ice-age orbit, eventually leading the moon to destroy life on our planet, ending with earth being kicked out of the solar system
    particles[i].x=center_x;       particles[i].y=center_y+296.0;  particles[i].size=5;   particles[i].mass=10.0;   particles[i].speed_x=0.008;      particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BLUE);       i+=1;
    particles[i].x=center_x;       particles[i].y=center_y;        particles[i].size=28;  particles[i].mass=2000.0; particles[i].speed_x=-6.715e-5;  particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(YELLOW);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+176.0;  particles[i].size=6;   particles[i].mass=11.0;   particles[i].speed_x=0.01;       particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(ORANGE);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+310.57; particles[i].size=1;   particles[i].mass=1.0;    particles[i].speed_x=0.01029;    particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=dotDrawingFunction;     particles[i].color=raylib::Color(WHITE);      i+=1;
    particles[i].x=center_x;       particles[i].y=center_y-50.0;   particles[i].size=3;   particles[i].mass=3.0;    particles[i].speed_x=-0.023;     particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(DARKPURPLE); i+=1;
    particles[i].x=center_x-451.0; particles[i].y=center_y;        particles[i].size=3;   particles[i].mass=3.8;    particles[i].speed_x=0.0;        particles[i].speed_y=0.0065915252310573;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(RED);        i+=1;
    particles[i].x=center_x+920.0; particles[i].y=center_y;        particles[i].size=15;  particles[i].mass=100.0;  particles[i].speed_x=0.0;        particles[i].speed_y=-0.0045;             particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BROWN);      i+=1;
    */    // jupiter experiments
    particles[i].x=center_x;       particles[i].y=center_y+296.0;  particles[i].size=5;   particles[i].mass=10.0;   particles[i].speed_x=0.008;      particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BLUE);       i+=1;
    particles[i].x=center_x;       particles[i].y=center_y;        particles[i].size=28;  particles[i].mass=2000.0; particles[i].speed_x=-6.715e-5;  particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(YELLOW);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+176.0;  particles[i].size=6;   particles[i].mass=11.0;   particles[i].speed_x=0.01;       particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(ORANGE);     i+=1;
    particles[i].x=center_x;       particles[i].y=center_y+310.57; particles[i].size=1;   particles[i].mass=1.0;    particles[i].speed_x=0.01029;    particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=dotDrawingFunction;     particles[i].color=raylib::Color(WHITE);      i+=1;
    particles[i].x=center_x;       particles[i].y=center_y-50.0;   particles[i].size=3;   particles[i].mass=3.0;    particles[i].speed_x=-0.023;     particles[i].speed_y=2.0840476894275e-04; particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(DARKPURPLE); i+=1;
    particles[i].x=center_x-451.0; particles[i].y=center_y;        particles[i].size=3;   particles[i].mass=3.8;    particles[i].speed_x=0.0;        particles[i].speed_y=0.0065915252310573;  particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(RED);        i+=1;
    particles[i].x=center_x+920.0; particles[i].y=center_y;        particles[i].size=15;  particles[i].mass=100.0;  particles[i].speed_x=0.0;        particles[i].speed_y=-0.0045;             particles[i].shapeDrawingFunction=ellipseDrawingFunction; particles[i].color=raylib::Color(BROWN);      i+=1;
    if (i > N_PARTICLES) {
        std::cout << "Not enougn N_PARTICLES. Needed: " << i << "; Actual: " << N_PARTICLES << ". Please increase it and try again.\n";
        return 1;
    }
    for (short j=i; j<N_PARTICLES; j++) {
        particles[j] = particles[i-1];
        particles[j].mass = 0.00000000000000000000001;
    }


    guiLoop(w);

	return 0;
}
