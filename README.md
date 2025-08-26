# Icons
We are using https://feathericons.com/ (Alternatives: search for "popular modern svg icon pack")
Icon files are of the form:
```svg
  <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="feather feather-activity"><polyline points="22 12 18 12 15 21 9 3 6 12 2 12"></polyline></svg>
```
Before saving, you should replace "currentColor" with #E0E0E0 for dark mode and #424242 for light mode.
You can also change stroke-width and size (replacing all occurrences of 24).
It icons are 32x32, which is slightly too large. Standard is 24.

-------------------------------------------------------------------------------
- [X] Center image when smaller than 
- [X] Compat: iterate/iterate_
- [X] Return RGB(r, g, b)
- [X] Enable/disable buttons
- [X] Params table
- [X] Thumbnail
- [X] Set parameter when switching to dspace
- [X] Tabulate color map
- [X] Maintain aspect ratio when selecting
- [X] Zoom in and out (ctrl-mousewheel), reset zoom
- [X] Obtain colormaps
- [X] Colormap changing
- [X] Colormap preview 
- [X] Sandbox
- [X] Iterate current point
- [X] Draw figure, iterate (1/2/3/...)
- [X] Built-in functions (loaded by default)
- [X] Functions tree
- [X] Compilation
- [X] Artifacts bug: QImage not thread-safe, use map()
- [X] Better tile scheduling / parallelization
- [X] Settings: mode, last function, window
- [X] Check all compiled functions against old version
- [X] Code editor
- [X] Show compiliation errors, click on error line

- [X] App icon https://doc.qt.io/qt-6/appicon.html
- [ ] Export image
- [ ] Printing
- [ ] Printf debugging

- [ ] Wider parameter section
- [ ] Setting thumbnail size has no effect
- [ ] Annotations: function vs temporary (orbit, Clear from index)
- [ ] Reset defaults

- [ ] Function tree manipulation
    - [ ] Add new file/folder
    - [ ] Delete file
    - [ ] Drag and drop to move to folder, change order


- [ ] Check all of Nuria's files
- [ ] Get better colormaps (3D effect?)

- [ ] Packaging/installer/signing
    - [ ] App store?
    - [ ] Github/Gitlab/gitea/codeberg - promote with Qt? Showcase
        - [ ] kurisuchan444 on github
    - [ ] Compute servers (other copies)
    - [ ] Update to 6.9
    - [ ] Gitlab/github
    - [ ] Windows
    - [ ] Linux
- [ ] Settings; files directory: platform-dependent - choosable

- [ ] Help
    - [ ] About menu (prepared, see Claude)
    - [ ] Explain how to get a compiler
    - [ ] Add how to debug
    - [ ] Manual: local web page
    - [ ] Help: Show cheatsheet from help
    - [ ] Help: Show long help as web page (integrated)
    - [ ] Add "what's new": continuous colormaps

---Major Features--------------------------------------------------------------
- [ ] Add your own colormap
- [ ] Notebook integration

---Minor Features--------------------------------------------------------------
- [ ] Dark mode
- [ ] Optimize tile sizes / subdivision
- [ ] Implement or eliminate forward

-------------------------------------------------------------------------------
# Cheatsheet
- Left drag: set selection
- Left click inside selection: move selection
- Left click outside selection: clear selection
- Mouse wheel down: zoom in (click to reset)
- Shift-left-drag: thumbnail (in parameter space only)
- Alt-left-drag: draw 
- 1-9: set orbit length
- 0: reset orbit


-------------------------------------------------------------------------------
# Refine Algorithm
Objectives:
    - Parallelize 
    - Preview every 100-250 ms
    - Keep cores busy even w/ different load per region
    - Cache-locality

Given:
    - pixels per second (per core)
    - picture size

Subdivide picture into small enough regions (16x16 - 32x32 pixels)
Do tiles in phases:
    - Phase 1 - single pix colors whole tile
    - Phase 2 - subdivide in 4 (3 more pixels)
    - Rest - calculate all.

Cache sizes: there are per-core caches, about 48k-128k.
An image is much larger, 3-8 MB. Memory organization is line-by line.
-> Forget about cache locality for now.

What if the tiles had their own memory that is mapped (it needs to be mapped
anyway) - this would give us better cache locality.

Results:
1000 x 1000 Mandi
single threaded: 765 (1307 pix/ms)
stripes 12 cores: 200 (5000 pix/ms)
Oldtiles 12 cores: 110 (9090 pix/ms)
Newtiles: 100 (10000 pix/ms)
    128x128     130
    64x64       100
    50x50       100 <-- keep at 50, multiple of sizes
    32x32       110
    16x16       170
-------------------------------------------------------------------------------
