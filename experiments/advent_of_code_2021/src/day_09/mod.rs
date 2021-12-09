struct Grid {
    fields: Vec<Vec<usize>>,
}
impl Grid {
    fn parse(input: &str) -> Self {
        let w = input.lines().next().unwrap().trim().len() + 2; // sentinels
        let mut fields = Vec::<Vec<usize>>::new();
        let mut sentinel = Vec::with_capacity(w);
        sentinel.resize(w, usize::MAX);
        fields.push(sentinel);
        for line in input.lines() {
            let f_line: Vec<_> = Some(usize::MAX)
                .iter()
                .copied()
                .chain(line.trim().chars().map(|c| (c as u8 - '0' as u8) as usize))
                .chain(Some(usize::MAX))
                .collect();
            assert_eq!(fields[0].len(), f_line.len());
            fields.push(f_line);
        }
        let mut sentinel = Vec::with_capacity(w);
        sentinel.resize(w, usize::MAX);
        fields.push(sentinel);
        Self { fields }
    }
    fn is_low_point(&self, x: usize, y: usize) -> bool {
        assert!(x > 0 && y > 0 && y < self.fields.len() - 1);
        let v = self.fields[y][x];
        let t = self.fields[y-1][x];
        let b = self.fields[y+1][x];
        let l = self.fields[y][x-1];
        let r = self.fields[y][x+1];
        v < t && v < b && v < l && v < r
    }
    fn sum_low_points(&self) -> usize {
        let mut c = 0;
        for y in 1..(self.fields.len() - 1) {
            for x in 1..(self.fields[y].len() - 1) {
                c += if self.is_low_point(x, y) { self.fields[y][x] + 1 } else { 0 };
            }
        }
        c
    }
}

fn part1() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("09.txt");
    let input = std::fs::read_to_string(input_path).unwrap();

    let res = Grid::parse(&input).sum_low_points();

    println!("day9/pt1: {}", res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("09.txt");
    let _input = std::fs::read_to_string(input_path).unwrap();

    // TODO

    println!("day9/pt2: {}", 0);
    Ok(())
}
pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test() {
        let input = "2199943210\n3987894921\n9856789892\n8767896789\n9899965678";
        assert_eq!(Grid::parse(&input).sum_low_points(), 15);
    }
}
