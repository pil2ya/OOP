import math
import tkinter as tk
from tkinter import messagebox


class EngineeringCalculator:

  def __init__(self, root):
    self.root = root
    self.root.title("스마트 공학용 계산기")
    self.root.geometry("350x520")
    self.root.minsize(350, 520)

    self.is_engineering_mode = False
    self.clear_on_next_input = False

    self.create_widgets()
    self.setup_layout()

  def create_widgets(self):
    # 결과 화면 (Entry)
    self.entry_var = tk.StringVar(value="0")
    self.edit = tk.Entry(
        self.root,
        textvariable=self.entry_var,
        font=("Arial", 18),
        justify="right",
        state="readonly",
    )

    # 공학용 모드 토글 버튼
    self.toggle_btn = tk.Button(
        self.root,
        text="공학용 ▶",
        font=("Arial", 11, "bold"),
        command=self.toggle_mode,
    )

    # 기본 버튼 정의 (일반용 20개)
    self.buttons_info = [
        ("C", self.on_clear),
        ("BS", self.on_backspace),
        ("(", lambda: self.on_input("(", True)),
        (")", lambda: self.on_input(")", True)),
        ("7", lambda: self.on_input("7", False)),
        ("8", lambda: self.on_input("8", False)),
        ("9", lambda: self.on_input("9", False)),
        ("/", lambda: self.on_input("/", True)),
        ("4", lambda: self.on_input("4", False)),
        ("5", lambda: self.on_input("5", False)),
        ("6", lambda: self.on_input("6", False)),
        ("*", lambda: self.on_input("*", True)),
        ("1", lambda: self.on_input("1", False)),
        ("2", lambda: self.on_input("2", False)),
        ("3", lambda: self.on_input("3", False)),
        ("-", lambda: self.on_input("-", True)),
        ("0", lambda: self.on_input("0", False)),
        (".", lambda: self.on_input(".", False)),
        ("=", self.on_equals),
        ("+", lambda: self.on_input("+", True)),
    ]

    self.buttons = []
    for text, cmd in self.buttons_info:
      btn = tk.Button(
          self.root, text=text, font=("Arial", 12, "bold"), command=cmd
      )
      self.buttons.append(btn)

    # 공학용 버튼 정의 (6개)
    self.eng_buttons_info = [
        ("sin", lambda: self.on_trig("sin")),
        ("cos", lambda: self.on_trig("cos")),
        ("tan", lambda: self.on_trig("tan")),
        ("√", self.on_sqrt),
        ("x^y", lambda: self.on_input("^", True)),
        ("π", lambda: self.on_input("π", True)),
    ]

    self.eng_buttons = []
    for text, cmd in self.eng_buttons_info:
      btn = tk.Button(
          self.root, text=text, font=("Arial", 11, "bold"), command=cmd
      )
      self.eng_buttons.append(btn)

  def setup_layout(self):
    # 기존 위젯들 배치 초기화
    for btn in self.buttons:
      btn.place_forget()
    for btn in self.eng_buttons:
      btn.place_forget()

    width = self.root.winfo_width()
    height = self.root.winfo_height()

    # 창 크기가 너무 작을 때 기본값 보정
    if width < 100:
      width = 350
    if height < 100:
      height = 520

    margin = 15
    spacing = 8

    self.edit.place(
        x=margin, y=margin, width=width - (margin * 2), height=40
    )
    self.toggle_btn.place(
        x=margin, y=65, width=width - (margin * 2), height=30
    )

    top_offset = 105
    normal_available_height = height - top_offset - margin - (spacing * 4)

    if self.is_engineering_mode:
      btn_width = (width - (margin * 2) - (spacing * 5)) // 6
      btn_height = normal_available_height // 5

      # 20개 기본 버튼 (4열 x 5행)
      for i, btn in enumerate(self.buttons):
        row = i // 4
        col = i % 4
        x = margin + col * (btn_width + spacing)
        y = top_offset + row * (btn_height + spacing)
        btn.place(x=x, y=y, width=btn_width, height=btn_height)

      # 6개 공학용 버튼 (우측에 2열 너비로 세로 배치)
      eng_x = margin + 4 * (btn_width + spacing)
      eng_height = normal_available_height // 6
      for i, btn in enumerate(self.eng_buttons):
        y = top_offset + i * (eng_height + (spacing // 2))
        btn.place(
            x=eng_x,
            y=y,
            width=(btn_width * 2) + spacing,
            height=eng_height,
        )
    else:
      available_width = width - (margin * 2) - (spacing * 3)
      btn_width = available_width // 4
      btn_height = normal_available_height // 5

      # 20개 기본 버튼 (4열 x 5행)
      for i, btn in enumerate(self.buttons):
        row = i // 4
        col = i % 4
        x = margin + col * (btn_width + spacing)
        y = top_offset + row * (btn_height + spacing)
        btn.place(x=x, y=y, width=btn_width, height=btn_height)

  def toggle_mode(self):
    self.is_engineering_mode = not self.is_engineering_mode
    if self.is_engineering_mode:
      self.root.geometry("560x520")
      self.toggle_btn.config(text="일반용 ◀")
    else:
      self.root.geometry("350x520")
      self.toggle_btn.config(text="공학용 ▶")
    self.root.update_idletasks()
    self.setup_layout()

  def on_clear(self):
    self.entry_var.set("0")
    self.clear_on_next_input = False

  def on_backspace(self):
    current = self.entry_var.get()
    if len(current) > 1 and current != "Error":
      self.entry_var.set(current[:-1])
    else:
      self.entry_var.set("0")

  def on_input(self, char, allow_chain):
    current = self.entry_var.get()

    if self.clear_on_next_input:
      if allow_chain:
        self.clear_on_next_input = False
      else:
        current = ""
        self.clear_on_next_input = False

    if current == "0" or current == "Error":
      self.entry_var.set(char)
    else:
      self.entry_var.set(current + char)

  def preprocess_expression(self, raw_expr):
    # 1단계: 숫자 바로 뒤에 π가 오는 경우 * 삽입 (예: 12π -> 12*π)
    step1 = ""
    for i in range(len(raw_expr)):
      step1 += raw_expr[i]
      if i + 1 < len(raw_expr):
        if raw_expr[i].isdigit() and raw_expr[i + 1] == "π":
          step1 += "*"

    # 2단계: π를 파이썬 수학 값으로 치환
    expanded = step1.replace("π", str(math.pi))

    # 3단계: 암묵적 곱셈 처리 (숫자( or )숫자 or )( )
    processed = ""
    for i in range(len(expanded)):
      processed += expanded[i]
      if i + 1 < len(expanded):
        curr = expanded[i]
        nxt = expanded[i + 1]
        if (
            (curr.isdigit() and nxt == "(")
            or (curr == ")" and nxt.isdigit())
            or (curr == ")" and nxt == "(")
        ):
          processed += "*"

    # 파이썬 거듭제곱 연산자 처리 (^)
    processed = processed.replace("^", "**")
    return processed

  def calculate(self):
    try:
      raw_expr = self.entry_var.get()
      expr = self.preprocess_expression(raw_expr)
      # 안전한 수식 계산을 위해 eval 사용
      result = eval(expr)
      return result
    except Exception:
      return "Error"

  def on_equals(self):
    result = self.calculate()
    if result == "Error":
      self.entry_var.set("Error")
    else:
      # 정수일 경우 소수점 제거 형식 정리
      if isinstance(result, float) and result.is_integer():
        result = int(result)
      self.entry_var.set(str(result))
    self.clear_on_next_input = True

  def on_sqrt(self):
    result = self.calculate()
    if result == "Error" or result < 0:
      self.entry_var.set("Error")
    else:
      val = math.sqrt(result)
      if val.is_integer():
        val = int(val)
      self.entry_var.set(str(val))
    self.clear_on_next_input = True

  def on_trig(self, func_name):
    result = self.calculate()
    if result == "Error":
      self.entry_var.set("Error")
    else:
      rad = math.radians(result)
      if func_name == "sin":
        res = math.sin(rad)
      elif func_name == "cos":
        res = math.cos(rad)
      elif func_name == "tan":
        res = math.tan(rad)

      # 미세한 부동소수점 오차 정리 (예: tan(90) 등 방지용 보정)
      if abs(res) < 1e-12:
        res = 0.0

      self.entry_var.set(str(res))
    self.clear_on_next_input = True


if __name__ == "__main__":
  root = tk.Tk()
  app = EngineeringCalculator(root)
  root.mainloop()