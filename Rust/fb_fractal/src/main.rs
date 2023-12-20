/*struct PrefixData<'a> {
    pub Prefix: &'a str,
}
struct VectorData<'a> {
    pub A: Vec<String>,
    pub B: Vec<String>,
    pub ACriteria: u32,
    pub BCriteria: u32,
    pub prefixData: PrefixData<'a>,
}

trait PrefixTrait<'a>  {
    fn getPrefixData(&self) -> &PrefixData<'a>;
    fn getMutPrefixData(&mut self) -> &mut PrefixData<'a>;
}

trait VectorTrait<'a>: PrefixTrait<'a> {

    fn getVectorData(&self) -> &VectorData<'a>;
    fn getMutVectorData(&mut self) -> &mut VectorData<'a>;

    fn flexibleInsert(prefixData: &PrefixTrait, AorB: &mut Vec<String>, message: &str) {
        let prefixData = prefixData.getPrefixData();
        let mut completeMessage: String = prefixData.Prefix.to_owned();
        completeMessage.push_str(message);
        AorB.push(completeMessage);
    }

    fn insert(&mut self, message: &str) {
        let vectorData = self.getMutVectorData();
        if message.len() as u32 % 2 == vectorData.ACriteria {
            Self::flexibleInsert(self.getPrefixData(), &mut vectorData.A, message);
        } else if message.len() as u32 % 2 == vectorData.BCriteria {
            Self::flexibleInsert(self.getPrefixData(), &mut vectorData.B, message);
        } else {
            panic!("No criteria to insert '{}'", message);
        }
    }
}


impl<'a> PrefixTrait<'a> for PrefixData<'a> {
    fn getPrefixData(&self) -> &PrefixData<'a> {&self}
    fn getMutPrefixData(&mut self) -> &mut PrefixData<'a> {self}
}

impl<'a> VectorTrait<'a> for VectorData<'a> {
    fn getVectorData(&self) -> &VectorData<'a> {&self}
    fn getMutVectorData(&mut self) -> &mut VectorData<'a> {self}
}
impl<'a> PrefixTrait<'a> for VectorData<'a> {
    fn getPrefixData(&self) -> &PrefixData<'a> {&self.prefixData}
    fn getMutPrefixData(&mut self) -> &mut PrefixData<'a> {&mut self.prefixData}
}

fn main() {
    let mut spike: VectorData = VectorData {
        A: vec![],
        B: vec![],
        ACriteria: 0,
        BCriteria: 1,
        prefixData: PrefixData {Prefix: "Spike: "},
    };
    spike.insert("I am cool!");
    spike.insert("I'm cool!");
}

*/

/*
struct Data<'a> {
    a: Vec<&'a str>,
    b: Vec<&'a str>,
    prefix: &'a str,
    count: u32,
}

enum InsertOrder { AthanB, BthanA }

fn doubleInsert<'a>(mut data: &mut Data<'a>, msg: &'a str, order: &InsertOrder) {
    let v1: &mut Vec<&'a str>;
    let v2: &mut Vec<&'a str>;
    match order {
        InsertOrder::AthanB => {
            v1 = &mut data.a;
            v2 = &mut data.b;
        }
        InsertOrder::BthanA => {
            v2 = &mut data.a;
            v1 = &mut data.b;
        }
    }
    v1.push(data.prefix);
    v1.push(msg);
    v2.push(msg);
    data.count += 1;
}

fn variableOrderInsertion<'a>(mut data: &mut Data<'a>, msg: &'a str) {
    if msg.len() % 2 == 0 {
        doubleInsert(&mut data, msg, &InsertOrder::AthanB);
    } else {
        doubleInsert(&mut data, msg, &InsertOrder::BthanA);
    }
}

fn main() {

    let mut data = Data {
        a: vec![],
        b: vec![],
        prefix: "For the first...",
        count: 0,
    };
    variableOrderInsertion(&mut data, "hello");
    variableOrderInsertion(&mut data, "rust");
}
*/

/*
use std::sync::Arc;

fn main() {
    let five = Arc::new(5);
    let seven = Arc::new(7);
    let mut eleven = Arc::new(10);
    *eleven+=1;


    println!("{} + {} = {}", five, seven, *five + *seven);
    println!("eleven is {}", *eleven);
}
 */

use std::fs::OpenOptions;
use std::path::PathBuf;
use memmap::{MmapOptions, MmapMut};
use std::io::Write;
use std::fs::File;

#[derive(Debug)]
struct ScreenInfo {
    width:      u32,
    height:     u32,
    bytespp:    u32,
    bpp:        u8,
    redShift:   u8,
    greenShift: u8,
    blueShift:  u8,
}

/// lap
const SCREEN: ScreenInfo = ScreenInfo {
    width:   1920,
    height:  1080,
    bytespp:    3,
    bpp:       24,
    redShift:  16,
    greenShift: 8,
    blueShift:  0,
};

/// returns both the 'mmap' and the 'screen' array -- 'mmap' sustains the reference used by 'screen'
/// TODO correct the lifetime association of the two variables
fn getFBByteArray() -> Result<(MmapMut, &'static mut [u8; (SCREEN.width*SCREEN.height*SCREEN.bytespp) as usize]), std::io::Error> {
    let mut mmap = unsafe { MmapOptions::new()
                                        .len((SCREEN.width*SCREEN.height*SCREEN.bytespp) as usize)
                                        .map_mut(&mut OpenOptions::new()
                                                                       .read(true)
                                                                       .write(true)
                                                                       .create(false)
                                                                       .open("/dev/fb0")?)? };
    let screen: &mut [u8; (SCREEN.width*SCREEN.height*SCREEN.bytespp) as usize] =
        unsafe { std::mem::transmute::< * mut u8, & mut [u8; (SCREEN.width*SCREEN.height*SCREEN.bytespp) as usize]>(mmap.as_mut_ptr()) };
    Ok((mmap, screen))
}

/// returns the 'mmap' for the framebuffer and the 'screen' as a 1D array
fn getFB1DPixelArray<PixelType>() -> Result<(MmapMut, &'static mut [PixelType;(SCREEN.width * SCREEN.height) as usize]), std::io::Error> {
    let (mut mmap, _) = getFBByteArray()?;
    let screen: &mut [PixelType;(SCREEN.width * SCREEN.height) as usize] =
        unsafe { std::mem::transmute::< * mut u8, &mut [PixelType;(SCREEN.width * SCREEN.height) as usize]>(mmap.as_mut_ptr()) };
    Ok((mmap, screen))
}

/// returns the 'mmap' for the framebuffer and the 'screen' as a 2D array: 'screen[y]['x]'
fn getFB2DPixelArray<PixelType>() -> Result<(MmapMut, &'static mut [[PixelType;SCREEN.width as usize];SCREEN.height as usize]), std::io::Error> {
    let (mut mmap, _) = getFBByteArray()?;
    let screen: &mut [[PixelType;SCREEN.width as usize];SCREEN.height as usize] =
        unsafe { std::mem::transmute::< * mut u8, &mut [[PixelType;SCREEN.width as usize];SCREEN.height as usize]>(mmap.as_mut_ptr()) };
    Ok((mmap, screen))
}

#[inline]
fn getColorAsPixel<PixelType>(red: u8, green: u8, blue: u8) -> PixelType
    where PixelType: std::convert::From<u8> + std::convert::From<u16> + std::convert::From<u32> +
          std::ops::Shl<PixelType, Output = PixelType> +
          std::ops::BitOr<PixelType, Output = PixelType> {
    PixelType::from(red)   << PixelType::from(SCREEN.redShift)   |
    PixelType::from(green) << PixelType::from(SCREEN.greenShift) |
    PixelType::from(blue)  << PixelType::from(SCREEN.blueShift)
}

/// paints a mandelbrot scene on a 1D byte array representing all the pixels on the screen
fn mandelbrot(frame: &'static mut [u8; (SCREEN.width*SCREEN.height*SCREEN.bytespp) as usize]) {
    let w = SCREEN.width;
    let h = SCREEN.height;
    let bytespp = SCREEN.bytespp;
    let line_length = w*bytespp;

    for (r, line) in frame.chunks_mut(line_length as usize).enumerate() {
        for (c, p) in line.chunks_mut(bytespp as usize).enumerate() {
            let x0 = ((c+0) as f32 / w as f32) * 3.5 - 2.5;
            let y0 = (r as f32 / h as f32) * 2.0 - 1.0;

            let mut it = 0;
            let max_it = 200;

            let mut x = 0.0;
            let mut y = 0.0;

            while x * x + y * y < 4.0 && it < max_it {
                let xtemp = x * x - y * y + x0;
                y = 2.0 * x * y + y0;
                x = xtemp;
                it += 1;
            }

            p[0] = (255.0f32 * (it as f32 / max_it as f32)) as u8;
            p[1] = (255.0f32 * (it as f32 / max_it as f32)) as u8;
            p[2] = (255.0f32 * (it as f32 / max_it as f32)) as u8;
        }
    }
}

fn X<PixelType>(screen: &'static mut [[PixelType;SCREEN.width as usize];SCREEN.height as usize])
    where PixelType: std::convert::From<u8> + std::convert::From<u16> + std::convert::From<u32> +
          std::ops::Shl<PixelType, Output = PixelType> +
          std::ops::BitOr<PixelType, Output = PixelType> {

    let vy = SCREEN.height as f32 / SCREEN.width as f32;
    //let vy = 0.09f32;
    let mut y = 0f32;
    for x in 0..SCREEN.width {
        screen [y as usize ] [x as usize] = getColorAsPixel::<PixelType>(
            ((x as u32) % 256) as u8,
            ((y as u32) % 256) as u8,
            ((x+y as u32) % 256) as u8);
        y += vy;
    }
}

fn Xmain() -> Result<(), std::io::Error> {
    let (mmap, screen) = getFB2DPixelArray::<u32>()?;
    println!("Starting screen: {:#?}", SCREEN);
    println!("MMapped Frame Buffer has {} pixels", screen.len());

    println!("Starting X...");
    X::<u32>(screen);

    Ok(())
}

fn main() -> Result<(), std::io::Error> {
    let (mmap, screen) = getFBByteArray()?;
    println!("Starting screen: {:#?}", SCREEN);
    println!("MMapped Frame Buffer has {} pixels", screen.len());
    println!("Starting Mandelbrot...");
    mandelbrot(screen);
    Ok(())
}

fn _main() -> Result<(), std::io::Error> {
    let (mmap, screen) = getFB2DPixelArray::<u32>()?;
    println!("Starting screen: {:#?}", SCREEN);
    println!("MMapped Frame Buffer has {} pixels", screen.len());

    let color = getColorAsPixel::<u32>(255, 128, 17);
    for y in 0..SCREEN.height {
        for x in 0..SCREEN.width {
            screen[y as usize][x as usize] = getColorAsPixel::<u32>(
            ((y as u32) % 256) as u8,
            ((0 as u32 + 0 as u32) % 256) as u8,
            ((x as f32 / SCREEN.width as f32 * 255f32) as u32 % 256) as u8);
            //std::thread::sleep_ms(1);
        }
    }
/*    for r in 0..255 {
    for pixel in screen.iter_mut() {
        *pixel = getColorAsPixel::<u32>(0,0,255);
        c += 1;
/*        *pixel = getColorAsPixel::<u32>(
            ((c /*+ r*/ as u32) % 256) as u8,
            ((c /*+ g*/ as u32) % 256) as u8,
            ((c /*+ b*/ as u32) % 256) as u8);
//        c += 1;
//        r = ((r as u32 + 1 as u32) % 256) as u8;
//        b = ((b as u32 + 1 as u32) % 256) as u8;
//        g = ((g as u32 + 1 as u32) % 256) as u8;*/
    }
}*/
    Ok(())
}
