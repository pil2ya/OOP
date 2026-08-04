import csv
import os
import tkinter as tk
from tkinter import messagebox, ttk


class Book:

  def __init__(self, book_id, title, author, is_borrowed=False):
    self.book_id = book_id
    self.title = title
    self.author = author
    self.is_borrowed = (
        is_borrowed
        if isinstance(is_borrowed, bool)
        else (is_borrowed == "True" or is_borrowed == "1")
    )


class LibraryApp:

  def __init__(self, root):
    self.root = root
    self.root.title("도서관 관리 프로그램")
    self.root.geometry("700x410")
    self.root.resizable(False, False)

    self.filename = "library.csv"
    self.books = []
    self.displayed_indices = []

    # 스타일 설정 (맑은 고딕)
    self.style = ttk.Style()
    self.style.configure(
        "Treeview.Heading", font=("Malgun Gothic", 10, "bold")
    )
    self.style.configure("Treeview", font=("Malgun Gothic", 10), rowheight=24)

    self.create_widgets()
    self.load_from_file()
    self.refresh_all()

  def create_widgets(self):
    # 좌우 마진 기준: Left = 20, Right = 680 (너비 660)

    # 1. 리스트뷰 (높이 210)
    columns = ("id", "title", "author", "status")
    self.tree = ttk.Treeview(self.root, columns=columns, show="headings")

    self.tree.heading("id", text="도서 번호")
    self.tree.heading("title", text="제목")
    self.tree.heading("author", text="저자")
    self.tree.heading("status", text="상태")

    self.tree.column("id", width=110, anchor="center")
    self.tree.column("title", width=230, anchor="w")
    self.tree.column("author", width=190, anchor="w")
    self.tree.column("status", width=130, anchor="center")

    self.tree.place(x=20, y=15, width=660, height=210)

    # 스크롤바 추가
    scrollbar = ttk.Scrollbar(
        self.root, orient="vertical", command=self.tree.yview
    )
    self.tree.configure(yscrollcommand=scrollbar.set)
    scrollbar.place(x=663, y=15, height=210)

    # 2. 입력 폼 영역 (y = 240)
    tk.Label(
        self.root, text="도서 번호:", font=("Malgun Gothic", 10)
    ).place(x=20, y=240)
    self.entry_id = tk.Entry(self.root, font=("Malgun Gothic", 10))
    self.entry_id.place(x=90, y=240, width=70, height=24)

    tk.Label(self.root, text="제목:", font=("Malgun Gothic", 10)).place(
        x=178, y=240
    )
    self.entry_title = tk.Entry(self.root, font=("Malgun Gothic", 10))
    self.entry_title.place(x=215, y=240, width=200, height=24)

    tk.Label(self.root, text="저자:", font=("Malgun Gothic", 10)).place(
        x=433, y=240
    )
    self.entry_author = tk.Entry(self.root, font=("Malgun Gothic", 10))
    self.entry_author.place(x=470, y=240, width=210, height=24)

    # 3. 도서 등록 버튼 (y = 275)
    btn_add = tk.Button(
        self.root,
        text="도서 등록",
        font=("Malgun Gothic", 10, "bold"),
        command=self.add_book,
    )
    btn_add.place(x=20, y=275, width=660, height=30)

    # 4. 검색 영역 (안내 텍스트 플레이스홀더 적용)
    self.search_placeholder = "🔍 검색할 도서 제목을 입력하세요..."
    self.entry_search = tk.Entry(self.root, font=("Malgun Gothic", 10))
    self.entry_search.place(x=20, y=318, width=460, height=26)

    # 플레이스홀더 이벤트 바인딩
    self.entry_search.insert(0, self.search_placeholder)
    self.entry_search.config(fg="grey")
    self.entry_search.bind("<FocusIn>", self.on_search_focus_in)
    self.entry_search.bind("<FocusOut>", self.on_search_focus_out)

    btn_search = tk.Button(
        self.root,
        text="검색",
        font=("Malgun Gothic", 10),
        command=self.search_books,
    )
    btn_search.place(x=490, y=318, width=90, height=26)

    btn_reset = tk.Button(
        self.root,
        text="전체보기",
        font=("Malgun Gothic", 10),
        command=self.reset_search,
    )
    btn_reset.place(x=590, y=318, width=90, height=26)

    # 5. 하단 기능 버튼 4개 (y = 358)
    btn_borrow = tk.Button(
        self.root,
        text="도서 대출",
        font=("Malgun Gothic", 10),
        command=lambda: self.update_status(borrow=True),
    )
    btn_borrow.place(x=20, y=358, width=150, height=34)

    btn_return = tk.Button(
        self.root,
        text="도서 반납",
        font=("Malgun Gothic", 10),
        command=lambda: self.update_status(borrow=False),
    )
    btn_return.place(x=190, y=358, width=150, height=34)

    btn_delete = tk.Button(
        self.root,
        text="도서 삭제",
        font=("Malgun Gothic", 10),
        command=self.delete_book,
    )
    btn_delete.place(x=360, y=358, width=150, height=34)

    btn_delete_all = tk.Button(
        self.root,
        text="전체 삭제",
        font=("Malgun Gothic", 10, "bold"),
        fg="red",
        command=self.delete_all_books,
    )
    btn_delete_all.place(x=530, y=358, width=150, height=34)

    # 6. Tab 키 포커스 이동 바인딩 설정
    self.entry_id.bind(
        "<Tab>", lambda event: self.focus_next(self.entry_title)
    )
    self.entry_title.bind(
        "<Tab>", lambda event: self.focus_next(self.entry_author)
    )
    self.entry_author.bind("<Tab>", lambda event: self.focus_next(self.entry_id))

  def on_search_focus_in(self, event):
    if self.entry_search.get() == self.search_placeholder:
      self.entry_search.delete(0, tk.END)
      self.entry_search.config(fg="black")

  def on_search_focus_out(self, event):
    if not self.entry_search.get().strip():
      self.entry_search.insert(0, self.search_placeholder)
      self.entry_search.config(fg="grey")

  def focus_next(self, next_widget):
    next_widget.focus_set()
    return "break"

  def load_from_file(self):
    if not os.path.exists(self.filename):
      return
    try:
      with open(
          self.filename, mode="r", encoding="utf-8-sig", newline=""
      ) as file:
        reader = csv.reader(file)
        for row in reader:
          if len(row) >= 4:
            b_id = int(row[0])
            title = row[1]
            author = row[2]
            is_borrowed = row[3] == "True"
            self.books.append(Book(b_id, title, author, is_borrowed))
    except Exception as e:
      messagebox.showerror("오류", f"파일을 읽는 중 오류가 발생했습니다: {e}")

  def save_to_file(self):
    try:
      with open(
          self.filename, mode="w", encoding="utf-8-sig", newline=""
      ) as file:
        writer = csv.writer(file)
        for book in self.books:
          writer.writerow(
              [book.book_id, book.title, book.author, book.is_borrowed]
          )
    except Exception as e:
      messagebox.showerror("오류", f"파일을 저장하는 중 오류가 발생했습니다: {e}")

  def refresh_list_view(self, indices_to_display):
    for item in self.tree.get_children():
      self.tree.delete(item)

    self.displayed_indices = indices_to_display
    for idx in self.displayed_indices:
      book = self.books[idx]
      status_str = "대출 중" if book.is_borrowed else "대출 가능"
      self.tree.insert(
          "",
          "end",
          values=(book.book_id, book.title, book.author, status_str),
      )

  def refresh_all(self):
    all_indices = list(range(len(self.books)))
    self.refresh_list_view(all_indices)

  def add_book(self):
    id_str = self.entry_id.get().strip()
    title = self.entry_title.get().strip()
    author = self.entry_author.get().strip()

    if not id_str or not title or not author:
      messagebox.showwarning("경고", "모든 정보를 입력해주세요.")
      return

    if not id_str.isdigit():
      messagebox.showwarning("경고", "도서 번호는 숫자만 입력 가능합니다.")
      return

    book_id = int(id_str)
    for b in self.books:
      if b.book_id == book_id:
        messagebox.showwarning("경고", "이미 존재하는 도서 번호입니다.")
        return

    self.books.append(Book(book_id, title, author, False))
    self.save_to_file()
    self.refresh_all()

    self.entry_id.delete(0, tk.END)
    self.entry_title.delete(0, tk.END)
    self.entry_author.delete(0, tk.END)
    self.entry_id.focus_set()
    messagebox.showinfo("성공", "도서가 등록되었습니다.")

  def search_books(self):
    keyword = self.entry_search.get().strip()
    if not keyword or keyword == self.search_placeholder:
      self.refresh_all()
      return

    filtered_indices = []
    for i, book in enumerate(self.books):
      if keyword in book.title:
        filtered_indices.append(i)
    self.refresh_list_view(filtered_indices)

  def reset_search(self):
    self.entry_search.delete(0, tk.END)
    self.entry_search.insert(0, self.search_placeholder)
    self.entry_search.config(fg="grey")
    self.refresh_all()

  def update_status(self, borrow):
    selected_items = self.tree.selection()
    if not selected_items:
      messagebox.showwarning("경고", "목록에서 도서를 선택해주세요.")
      return

    item_id = selected_items[0]
    selected_index = self.tree.index(item_id)
    real_index = self.displayed_indices[selected_index]
    target_book = self.books[real_index]

    if borrow:
      if target_book.is_borrowed:
        messagebox.showinfo("알림", "이미 대출 중인 도서입니다.")
      else:
        target_book.is_borrowed = True
        self.save_to_file()
        self.refresh_list_view(self.displayed_indices)
        messagebox.showinfo("성공", "대출이 완료되었습니다.")
    else:
      if not target_book.is_borrowed:
        messagebox.showinfo("알림", "대출 중이 아닌 도서입니다.")
      else:
        target_book.is_borrowed = False
        self.save_to_file()
        self.refresh_list_view(self.displayed_indices)
        messagebox.showinfo("성공", "반납이 완료되었습니다.")

  def delete_book(self):
    selected_items = self.tree.selection()
    if not selected_items:
      messagebox.showwarning("경고", "목록에서 도서를 선택해주세요.")
      return

    item_id = selected_items[0]
    selected_index = self.tree.index(item_id)
    real_index = self.displayed_indices[selected_index]

    del self.books[real_index]
    self.save_to_file()
    self.refresh_all()
    messagebox.showinfo("성공", "도서가 삭제되었습니다.")

  def delete_all_books(self):
    if not self.books:
      messagebox.showinfo("알림", "삭제할 도서가 없습니다.")
      return

    if messagebox.askyesno("경고", "정말 모든 도서를 삭제하시겠습니까?"):
      self.books.clear()
      self.save_to_file()
      self.refresh_all()
      messagebox.showinfo("성공", "모든 도서가 삭제되었습니다.")


if __name__ == "__main__":
  root = tk.Tk()
  app = LibraryApp(root)
  root.mainloop()