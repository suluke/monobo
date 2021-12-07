#![deny(clippy::all)]

mod day_01;
use day_01::solve as day_01;
mod day_02;
use day_02::solve as day_02;
mod day_03;
use day_03::solve as day_03;
mod day_04;
use day_04::solve as day_04;
mod day_05;
use day_05::solve as day_05;

fn main() -> std::io::Result<()> {
    day_01()?;
    day_02()?;
    day_03()?;
    day_04()?;
    day_05()
}
