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
mod day_06;
use day_06::solve as day_06;
mod day_07;
use day_07::solve as day_07;
mod day_08;
use day_08::solve as day_08;
mod day_09;
use day_09::solve as day_09;
mod day_10;
use day_10::solve as day_10;

fn main() -> std::io::Result<()> {
    day_01()?;
    day_02()?;
    day_03()?;
    day_04()?;
    day_05()?;
    day_06()?;
    day_07()?;
    day_08()?;
    day_09()?;
    day_10()
}
