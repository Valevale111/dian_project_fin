//使用ANSI打印图片的尝试,RGB转灰度图


export const getAscii = (data: Uint8ClampedArray, index: number) => {
  const r = data[index];
  const g = data[index + 1];
  const b = data[index + 2];
  const a = data[index + 3];

  const i = (0.2126 * r + 0.7152 * g + 0.0722 * b);

  return a !== 0 ? pixel[Math.floor(i / weight)] : pixel[0];
};
