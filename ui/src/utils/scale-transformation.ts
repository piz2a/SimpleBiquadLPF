// 로그 스케일 변환 수식
export const linearToLog = (p: number, min: number, max: number) =>
  min * Math.pow(max / min, p);

export const logToLinear = (f: number, min: number, max: number) =>
  Math.log(f / min) / Math.log(max / min);
