<!-- 문서 맨 처음에 위치하는 최상단 앵커 -->
<a name="top"></a>

# 객체지향 정보
## Python & C++ 를 이용한 Programing

![Markdown](./IMG/markdown.png)

# 📝 GitHub README 마크다운 작성 가이드

GitHub의 `README.md`는 프로젝트를 소개하고 설명하는 얼굴입니다. 가독성을 높여주는 핵심 마크다운 문법과 팁을 정리했습니다.

---

## 📌 목차 (Table of Contents)
* [1. 제목 (Headings)](#heading)
* [2. 텍스트 강조 및 스타일 (Text Styling)](#style)
* [3. 목록 및 리스트 (Lists)](#list)
* [4. 링크 및 이미지 (Links & Images)](#link)
* [5. 코드 블록 (Code Blocks)](#code)
* [6. 이미지 및 콘텐츠 정렬 (HTML 태그 활용)](#align)
* [7. 구분선 (Horizontal Rule)](#hr)

---

## 💡 목차 작성법 안내
문서가 길어질 때 독자가 원하는 위치로 한 번에 이동할 수 있도록 만들어주는 **페이지 내부 링크(Anchor Link)** 기능입니다.<br>
깃허브에서는 대제목을 소문자로 바꾸고 띄어쓰기는 하이픈(`-`)으로 연결하면 자동으로 링크가 연결됩니다.

    * [1. 제목 (Headings)](#1--제목-headings)
    * [2. 텍스트 강조 및 스타일 (Text Styling)](#2--텍스트-강조-및-스타일-text-styling)
    * [3. 목록 및 리스트 (Lists)](#3--목록-및-리스트-lists)
    * [4. 링크 및 이미지 (Links & Images)](#4--링크-및-이미지-links--images)
    * [5. 코드 블록 (Code Blocks)](#5--코드-블록-code-blocks)
    * [6. 이미지 및 콘텐츠 정렬 (HTML 태그 활용)](#6--이미지-및-콘텐츠-정렬-html-태그-활용)
    * [7. 구분선 (Horizontal Rule)](#7--구분선-horizontal-rule)

---

<a name="heading"></a>
## 1. 📑 제목 (Headings)
문서의 구조를 나누는 단계별 제목입니다. `#` 개수에 따라 크기가 달라집니다. (메인 제목부터 소제목까지 순서대로 사용)

    # 🚀 메인 제목 (H1) - 보통 프로젝트 이름
    ## 📌 주요 섹션 (H2) - 대주제 (예: 개발 과정)
    ### 🛠️ 하위 항목 (H3) - 소주제 (예: 트러블슈팅)
    #### 세부 항목 (H4)

<div align="right">
  <a href="#top"><b>👆 맨 위로 이동하기</b></a>
</div>

---

<a name="style"></a>
## 2. ✍️ 텍스트 강조 및 스타일 (Text Styling)
중요한 키워드나 코드, 인용구를 강조할 때 사용합니다.

    * **굵게 강조** 또는 __굵게 강조__
    * *기울임* 또는 _기울임_
    * ~~취소선~~
    * `인라인 코드` (문장 안에서 코드나 파일명을 적을 때)
    * > 중요한 노트나 팁, 인용문을 넣을 때 사용합니다.

<div align="right">
  <a href="#top"><b>👆 맨 위로 이동하기</b></a>
</div>

---

<a name="list"></a>
## 3. 📋 목록 및 리스트 (Lists)
항목을 깔끔하게 나열할 때 사용합니다. 들여쓰기(`Tab` 또는 스페이스바 4칸)를 사용하면 하위 목록을 만들 수 있습니다.

    * 일반 글머리 기호 1
    * 일반 글머리 기호 2
      * 하위 글머리 기호 A
      * 하위 글머리 기호 B

    1. 번호가 매겨진 목록 1
    2. 번호가 매겨진 목록 2

<div align="right">
  <a href="#top"><b>👆 맨 위로 이동하기</b></a>
</div>

---

<a name="link"></a>
## 4. 🔗 링크 및 이미지 (Links & Images)
웹사이트 링크를 걸거나, 로컬/외부 이미지를 삽입할 수 있습니다.

    [텍스트 링크 이름](https://github.com)

    ![이미지 설명(Alt Text)](./image_path.png)

<div align="right">
  <a href="#top"><b>👆 맨 위로 이동하기</b></a>
</div>

---

<a name="code"></a>
## 5. 💻 코드 블록 (Code Blocks)
작성한 소스 코드나 터미널 명령어(CLI)를 예쁘게 보여줄 때 사용합니다. 언어 이름을 적으면 문법 강조(Syntax Highlighting)가 적용됩니다.

    ```python
    # 파이썬 코드 예시
    def hello():
        print("Hello, GitHub!")
    ```

    ```cpp
    // C++ 코드 예시
    #include <iostream>
    using namespace std;
    ```

<div align="right">
  <a href="#top"><b>👆 맨 위로 이동하기</b></a>
</div>

---

<a name="align"></a>
## 6. 🖼️ 이미지 및 콘텐츠 정렬 (HTML 태그 활용)
마크다운 기본 문법만으로는 가운데 정렬이나 크기 조절이 어려울 때, HTML 태그를 섞어서 쓰면 완벽하게 제어할 수 있습니다.

### ① 이미지를 가운데 정렬하고 크기 조절하기
    <div align="center">
      <img src="./my_image.png" alt="Description" width="500">
    </div>

### ② 이미지를 가로로 나란히 배치하기 (테이블 활용)
    <div align="center">
      <table border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse; border: none; margin: 0 auto;">
        <tr>
          <td align="center" style="border: none; padding: 0 10px; vertical-align: top;">
            <p style="margin: 0 0 10px 0;"><b>버전 1</b></p>
            <img src="./image1.png" alt="v1" width="350">
          </td>
          <td align="center" style="border: none; padding: 0 10px; vertical-align: top;">
            <p style="margin: 0 0 10px 0;"><b>버전 2</b></p>
            <img src="./image2.png" alt="v2" width="350">
          </td>
        </tr>
      </table>
    </div>

<div align="right">
  <a href="#top"><b>👆 맨 위로 이동하기</b></a>
</div>

---

<a name="hr"></a>
## 7. ➖ 구분선 (Horizontal Rule)
섹션과 섹션을 시각적으로 깔끔하게 분리할 때 사용합니다. (줄 바꿈 효과)

    ---

    ---

<div align="right">
  <a href="#top"><b>👆 맨 위로 이동하기</b></a>
</div>
