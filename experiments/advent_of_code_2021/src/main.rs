#![deny(clippy::all)]

mod day_01;
use day_01::solve as day_01;
mod day_02;
use day_02::solve as day_02;

fn main() -> std::io::Result<()> {
    day_01()?;
    day_02()
}
