use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len()-2..]
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());
    let res = input.len(); // TODO
    println!("day{}/pt1: {}", day(), res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let input = get_input(day());
    let res = input.len(); // TODO
    println!("day{}/pt2: {}", day(), res);
    Ok(())
}
pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}
#[cfg(test)]
mod tests {
    #[test]
    fn test() {}
}
