from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parent
RAW = ROOT / "raw" / "howto"
OUT = ROOT / "generated"


def load_gif_samples(path: Path) -> list[Image.Image]:
    image = Image.open(path)
    count = getattr(image, "n_frames", 1)
    indices = sorted({0, max(0, count // 2), max(0, count - 1)})
    frames: list[Image.Image] = []
    for index in indices:
        image.seek(index)
        frames.append(image.convert("RGBA"))
    return frames


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    gifs = sorted(RAW.glob("*.gif"))
    if not gifs:
        raise SystemExit(f"no official GIFs found under {RAW}")

    thumb_w = 180
    thumb_h = 130
    label_h = 28
    cols = 3
    rows = len(gifs)
    sheet = Image.new("RGB", (cols * thumb_w, rows * (thumb_h + label_h)),
                      "white")
    draw = ImageDraw.Draw(sheet)

    for row, gif in enumerate(gifs):
      samples = load_gif_samples(gif)
      label = gif.stem.replace("images__howto__", "").replace(
          "images__main__howto__", "").replace("__", "/")
      for col in range(cols):
          x = col * thumb_w
          y = row * (thumb_h + label_h)
          draw.rectangle((x, y, x + thumb_w - 1, y + thumb_h + label_h - 1),
                         outline="black")
          frame = samples[min(col, len(samples) - 1)]
          frame.thumbnail((thumb_w - 8, thumb_h - 8), Image.Resampling.NEAREST)
          ox = x + (thumb_w - frame.width) // 2
          oy = y + 4
          sheet.paste(frame.convert("RGB"), (ox, oy))
          draw.text((x + 4, y + thumb_h + 2), label[:28], fill="black")

    out = OUT / "official_howto_gif_contact_sheet.png"
    sheet.save(out)
    print(f"wrote {out}")


if __name__ == "__main__":
    main()

