class LinkedList < Formula
  desc "C++ linked list demo with CSV loader - supports dark/light terminal themes"
  homepage "https://github.com/jguida941/LinkedList"
  url "https://github.com/jguida941/LinkedList/archive/refs/tags/v1.0.0.tar.gz"
  sha256 "c73a2851467e391b8fcc81548fc3ec72a2ec1b2741572892659a969e9fd2cd58"
  version "1.0.0"
  head "https://github.com/jguida941/LinkedList.git", branch: "main"

  depends_on "cmake" => :build

  def install
    # Source files are now in src/ directory
    system "cmake", "-S", ".", "-B", "build", "-DCMAKE_BUILD_TYPE=Release"
    system "cmake", "--build", "build", "--config", "Release"
    bin.install "build/Linked_List" => "linked-list"
    pkgshare.install Dir["data/*.csv"]
  end

  def caveats
    <<~EOS
      The linked-list program auto-detects your terminal theme (dark/light).
      To force a specific theme, set the COLOR_THEME environment variable:
        export COLOR_THEME=dark   # for dark terminals
        export COLOR_THEME=light  # for light terminals
        export COLOR_THEME=mono   # for no colors
    EOS
  end

  test do
    # Run interactively with a single input to exit (menu option 9)
    pipe_output("#{bin}/linked-list #{pkgshare}/eBid_Monthly_Sales.csv 98109", "9\n", 0)
  end
end
