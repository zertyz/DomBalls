use raylib::prelude::*;
use rand::Rng;
use std::sync::{Mutex, Arc};
use std::{thread, time};
use std::thread::JoinHandle;
use std::borrow::BorrowMut;


#[derive(Copy,Clone)]
enum Shape {
    Dot,
    Rectangle,
    Ellipse,
}

#[derive(Copy,Clone)]
struct Particle {
    x:     f64,
    y:     f64,
    size:  i32,
    sx:    f64,
    sy:    f64,
    mass:  f64,
    shape: Shape,
    color: Color,
}

//static gy : f32 = 0.0002;
//static gx : f32 = 0.0002;
static gy : f64 = 0.0;
static gx : f64 = 0.0;

//const N_PARTICLES : usize = 8192/*  * 35*/;
const N_PARTICLES : usize = 10;
const G : f64 = 0.0000001;

// this update function treats all particles as massless and contactless -- they do not exerce gravity nor colide
fn update_positions_phantom(particles: &mut [Particle; N_PARTICLES]) {

    for particle in particles.iter_mut() {

        let ix: i32 = particle.x as i32;
        let iy: i32 = particle.y as i32;

        // x boundaries
        if (ix + particle.size) > 1366 {
            if particle.sx > 0.0 {
                particle.sx = -particle.sx;
                particle.x = (1366 - particle.size) as f64;
            }
        } else if ix < 0 {
            if particle.sx < 0.0 {
                particle.sx = -particle.sx;
                particle.x = 0.0;
            }
        }

        // y boundaries
        if (iy + particle.size) > 768 {
            if particle.sy > 0.0 {
                particle.sy = -particle.sy;
                particle.y = (768 - particle.size) as f64;
            }
        } else if iy < 0 {
            if particle.sy < 0.0 {
                particle.sy = -particle.sy;
                particle.y = 0.0;
            }
        }

        particle.sy += gy;
        particle.sx += gx;

        particle.x += particle.sx;
        particle.y += particle.sy;
    }

}

fn get_accelerations_due_to_gravity(particle1: &Particle, particle2: &Particle) -> (f64, f64, f64, f64) {

//    let _G : f64 = ( 6.674184e-10_f64  * 5.972e24_f64.powi(2) ) / 300.0e6_f64.powi(2);  // G constant for units: distance: each pixel, 1 light-second and mass in multiples of the earth's
//    let G : f32 = 1.0;


    // the (quadratic) distances² involved between the two bodies
    let x_distance :  f64 = (particle2.x - particle1.x) as f64;
    let x_distance_2: f64 = x_distance.powi(2);
    let y_distance :  f64 = (particle2.y - particle1.y) as f64;
    let y_distance_2: f64 = y_distance.powi(2);
    let distance_2:   f64 = (x_distance_2 + y_distance_2);
    // force due to gravity
    let force: f64 = G * ((particle1.mass as f64 * particle2.mass as f64) / distance_2);
    // vectors
    let sin: f64 = (y_distance_2 / distance_2).sqrt();
    let cos: f64 = (x_distance_2 / distance_2).sqrt();
    // accelerations involved
    let acceleration1: f64 = force / particle1.mass as f64;
    let acceleration2: f64 = force / particle2.mass as f64;
    let x_acceleration1: f64 = acceleration1 * cos * if x_distance >= 0.0 {1.0} else {-1.0};
    let y_acceleration1: f64 = acceleration1 * sin * if y_distance >= 0.0 {1.0} else {-1.0};
    let x_acceleration2: f64 = acceleration2 * cos * if x_distance <= 0.0 {1.0} else {-1.0};
    let y_acceleration2: f64 = acceleration2 * sin * if y_distance <= 0.0 {1.0} else {-1.0};

    (x_acceleration1, y_acceleration1, x_acceleration2, y_acceleration2)
}

// this update function treats all particles as stars -- their sizes and masses are related and they attract each other... but they not collide, after all, we live in a 3D world... don't we?
unsafe fn update_positions_stars(particles: &mut [Particle; N_PARTICLES]) {

    let len : usize =  particles.len();

    let particles_ptr = particles.as_mut_ptr();

    for i in 0 .. len {

        let mut particle1 = &mut *particles_ptr.add(i);

        for j in i+1 .. len {

            let mut particle2 = &mut *particles_ptr.add(j);

//            let (x_acceleration1, y_acceleration1, x_acceleration2, y_acceleration2) = get_accelerations_due_to_gravity(&particle1, &particle2);

            // the (quadratic) distances² involved between the two bodies
            let x_distance :  f64 = (particle2.x - particle1.x) as f64;
            let x_distance_2: f64 = x_distance.powi(2);
            let y_distance :  f64 = (particle2.y - particle1.y) as f64;
            let y_distance_2: f64 = y_distance.powi(2);
            let distance_2:   f64 = (x_distance_2 + y_distance_2);
            // inelastic collision ?
            if (distance_2 <= ( particle1.size.pow(2) + particle2.size.pow(2) ) as f64 ) {
                let sx = (particle1.sx*particle1.mass + particle2.sx*particle2.mass) / (particle1.mass+particle2.mass);
                let sy = (particle1.sy*particle1.mass + particle2.sy*particle2.mass) / (particle1.mass+particle2.mass);
                particle1.sx = sx;
                particle1.sy = sy;
                particle2.sx = sx;
                particle2.sy = sy;
                continue;
            }
            // force due to gravity
            let force: f64 = G * ((particle1.mass as f64 * particle2.mass as f64) / distance_2);
            // vectors
            let sin: f64 = (y_distance_2 / distance_2).sqrt();
            let cos: f64 = (x_distance_2 / distance_2).sqrt();
            // accelerations involved
            let acceleration1: f64 = force / particle1.mass as f64;
            let acceleration2: f64 = force / particle2.mass as f64;
            let x_acceleration1: f64 = acceleration1 * cos * if x_distance >= 0.0 {1.0} else {-1.0};
            let y_acceleration1: f64 = acceleration1 * sin * if y_distance >= 0.0 {1.0} else {-1.0};
            let x_acceleration2: f64 = acceleration2 * cos * if x_distance <= 0.0 {1.0} else {-1.0};
            let y_acceleration2: f64 = acceleration2 * sin * if y_distance <= 0.0 {1.0} else {-1.0};

            // compute accelerations
            particle1.sx += x_acceleration1;
            particle1.sy += y_acceleration1;
            particle2.sx += x_acceleration2;
            particle2.sy += y_acceleration2;
        }

        // after computing all accelerations for particle1, we may update its position
/*        if (particle1.sx > 0.1) {
            particle1.sx = 0.1;
        } else if (particle1.sx < -0.1) {
            particle1.sx = -0.1;
        }
        if (particle1.sy > 0.1) {
            particle1.sy = 0.1;
        } else if (particle1.sy < -0.1) {
            particle1.sy = -0.1;
        }*/
            particle1.x += particle1.sx;
            particle1.y += particle1.sy;
//        particle1.x += 0.01;
//        particle1.y += 0.01;
    }

/*    for particle in particles.iter_mut() {

        let ix: i32 = particle.x as i32;
        let iy: i32 = particle.y as i32;

        // x boundaries
        if (ix + particle.size) > 1366 {
            if particle.sx > 0.0 {
                particle.sx = -particle.sx;
                particle.x = (1366 - particle.size) as f32;
            }
        } else if ix < 0 {
            if particle.sx < 0.0 {
                particle.sx = -particle.sx;
                particle.x = 0.0;
            }
        }

        // y boundaries
        if (iy + particle.size) > 768 {
            if particle.sy > 0.0 {
                particle.sy = -particle.sy;
                particle.y = (768 - particle.size) as f32;
            }
        } else if iy < 0 {
            if particle.sy < 0.0 {
                particle.sy = -particle.sy;
                particle.y = 0.0;
            }
        }

        particle.sy += gy;
        particle.sx += gx;

        particle.x += particle.sx;
        particle.y += particle.sy;
    }*/

}



fn gui(particles: &mut [Particle; N_PARTICLES]) {
    let (mut rl, thread) = raylib::init()
        .fullscreen()
        .size(1366, 768)
        .title("Hello, World")
        .vsync()
        .build();

    while !rl.window_should_close() {
        let mut d = rl.begin_drawing(&thread);

        d.clear_background(Color::BLACK);
//        d.clear_background(Color::new(rng.gen_range(0, 255), rng.gen_range(0, 255), rng.gen_range(0, 255), rng.gen_range(64, 192)));
//        d.draw_text("Hello, world!", 12, 12, 20, Color::BLACK);

//        update_positions_stars(particles);

        for particle in particles.iter_mut() {

            let ix: i32 = particle.x as i32;
            let iy: i32 = particle.y as i32;

            match particle.shape {
                Shape::Dot => {
                    d.draw_pixel(ix, iy, particle.color);
                },
                Shape::Rectangle => {
                    d.draw_rectangle(ix, iy, particle.size, particle.size, particle.color)
                },
                Shape::Ellipse   => {
                    let _ix = (particle.x + (particle.size as f64 / 2.0)) as i32;
                    let _iy = (particle.y + (particle.size as f64 / 2.0)) as i32;
                    d.draw_circle(_ix, _iy, particle.size as f32, particle.color)
                },
            }

        }

        d.draw_fps(10, 10);
    }

}

static mut _particles : [Particle; N_PARTICLES] = [Particle {x:0.0, y:0.0, size:0, mass:0.0, sx:0.0, sy:0.0, shape:Shape::Ellipse, color:Color::BLACK}; N_PARTICLES];

unsafe fn _main() {

    let mut rng = rand::thread_rng();

    let mut particles: &mut [Particle; N_PARTICLES] = &mut _particles;

    // random size
/*    for particle in particles.iter_mut() {
        particle.x     = 700.0;
        particle.y     = 100.0;
        particle.size  = rng.gen_range(5, 30);
        particle.mass  = 1.0;
        particle.sx    = rng.gen_range(-10.0, 10.0);
        particle.sy    = rng.gen_range(-10.0, 10.0);
        particle.shape = if particle.size > 17 {Shape::Ellipse} else {Shape::Rectangle};
        particle.color = Color::new(rng.gen_range(0, 255), rng.gen_range(0, 255), rng.gen_range(0, 255), rng.gen_range(64, 192));
    }
*/
    // 1 pixel rectangles
    for particle in particles.iter_mut() {
        particle.x     = rng.gen_range(0.0, 1365.0);
        particle.y     = rng.gen_range(0.0, 767.0);
//        particle.x     = 550.0;
//        particle.y     = 200.0;
        particle.size  = rng.gen_range(5, 10);
        particle.mass  = rng.gen_range(1.0, 1000.0);
        particle.size  = 1 + ((particle.mass-1.0)*0.01) as i32;
        particle.sx    = rng.gen_range(-0.001, 0.001);
        particle.sy    = rng.gen_range(-0.001, 0.001);
//        particle.sx    = 0.0;
//        particle.sy    = 0.0;
        particle.shape = Shape::Ellipse;
        particle.color = Color::new(rng.gen_range(128, 255), rng.gen_range(128, 255), rng.gen_range(128, 255), rng.gen_range(64, 192));
        particle.size = 0;
    }

    // planets
    let mut i : usize = 0;
    particles[i].x=1366.0/2.0; particles[i].y=680.0;     particles[i].size=5;   particles[i].mass=10.0;   particles[i].sx=0.0008;      particles[i].sy=0.0; particles[i].shape=Shape::Ellipse; particles[i].color=Color::BLUE;       i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=768.0/2.0; particles[i].size=28;  particles[i].mass=2000.0; particles[i].sx=-0.00000546; particles[i].sy=0.0; particles[i].shape=Shape::Ellipse; particles[i].color=Color::YELLOW;     i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=560.0;     particles[i].size=6;   particles[i].mass=11.0;   particles[i].sx=0.001;       particles[i].sy=0.0; particles[i].shape=Shape::Ellipse; particles[i].color=Color::ORANGE;     i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=694.5;     particles[i].size=1;   particles[i].mass=1.0;    particles[i].sx=0.001030;    particles[i].sy=0.0; particles[i].shape=Shape::Ellipse; particles[i].color=Color::WHITE;      i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=334.0;     particles[i].size=3;   particles[i].mass=3.0;    particles[i].sx=-0.0022;     particles[i].sy=0.0; particles[i].shape=Shape::Ellipse; particles[i].color=Color::DARKPURPLE; i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=2.0;       particles[i].size=3;   particles[i].mass=3.8;    particles[i].sx=-0.000729;   particles[i].sy=0.0; particles[i].shape=Shape::Ellipse; particles[i].color=Color::RED;        i+=1;
    for j in i..N_PARTICLES {
        particles[j] = particles[i-1];
        particles[j].mass = 0.000000000000000001;
    }

    // earth test
/*    i=0;
    particles[i].x=1366.0/2.0; particles[i].y=680.0;     particles[i].size=5;   particles[i].mass=10.0;   particles[i].sx=0.0008-0.0008;     particles[i].sy=0.0; particles[i].shape=Shape::Ellipse; particles[i].color=Color::BLUE;   i+=1;
    particles[i].x=1366.0/2.0; particles[i].y=701.0;     particles[i].size=1;   particles[i].mass=1.0;    particles[i].sx=0.001030-0.0008;   particles[i].sy=0.0; particles[i].shape=Shape::Ellipse; particles[i].color=Color::WHITE;  i+=1;
    for j in i..N_PARTICLES {
        particles[j] = particles[i-1];
        particles[j].mass = 0.000000000000000001;
    }*/

    let update_thread : JoinHandle<()>;
    {
        update_thread = thread::spawn(move || {
            loop {
                update_positions_stars(&mut _particles);
//                std::thread::sleep(time::Duration::from_nanos(1));
            }
        });
    }

    gui(&mut _particles);

    //update_thread.join();

}

// run
fn main() {
    unsafe {_main();}
}

// test
fn __main() {
    let particle1: Particle = Particle {x: 0.0, y:0.0, size:1, mass:1.0, sx:0.0, sy:0.0, shape:Shape::Ellipse, color:Color::BLACK};
    let particle2: Particle = Particle {x:1.0, y:1.0, size:1, mass:2.0, sx:0.0, sy:0.0, shape:Shape::Ellipse, color:Color::BLACK};

    let (ax1, ay1, ax2, ay2) = get_accelerations_due_to_gravity(&particle2, &particle1);

    println!("Acceleration vectors: v1 = ({},{}); v2 = ({},{})", ax1, ay1, ax2, ay2);
}