#![deny(clippy::all)]

fn get_input(day: &str) -> String {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push(format!("{}.txt", day));
    std::fs::read_to_string(input_path).unwrap()
}

macro_rules! submods {
    ($m:ident) => {
        mod $m;
    };
    ($m:ident, $($ms:ident),+) => {
        mod $m;
        submods!($($ms),+);
    };
}

submods!(
    day_01, day_02, day_03, day_04, day_05, day_06, day_07, day_08, day_09, day_10, day_11, day_12,
    day_13, day_14, day_15, day_16, day_17, day_18, day_19, day_20, day_21, day_22, day_23, day_24
);

macro_rules! solve {
    ($day:ident) => {
        $day::solve()?;
    };
    ($day:ident, $($days:ident),+) => {
        $day::solve()?;
        solve!($($days),+);
    };
}

fn main() -> std::io::Result<()> {
    solve!(
        day_01, day_02, day_03, day_04, day_05, day_06, day_07, day_08, day_09, day_10, day_11,
        day_12, day_13, day_14, day_15, day_16, day_17, day_18, day_19, day_20, day_21, day_22,
        day_23, day_24
    );
    Ok(())
}
