/*
 * ============================================================================
 * 控制台光线追踪体素渲染器 (Console Voxel Ray Tracer)
 * ============================================================================
 *
 * 【概述】
 *   本程序在 Windows 控制台内实现了一个实时交互的光线追踪体素引擎。
 *   用户可以在三维方块世界中自由漫游、放置/删除方块、旋转方块朝向，
 *   并实时观察包含动态天空、镜面反射、半透明等效果的渲染画面。
 *   支持彩色 ANSI 像素输出与 ASCII 风格字符画两种显示模式，
 *   并提供了完整的存档、读档、截图功能。
 *
 * 【功能模块】
 *  1. 渲染核心
 *	 - 递归光线追踪：对每个像素发射射线，在体素世界中递归追踪。
 *	 - 几何求交：
 *	   * 普通方块：基于 DDA 算法的快速 AABB 体素遍历。
 *	   * 特殊方块：支持三角形面片 (Moller–Trumbore)、球面、双线性曲面片。
 *	   * 镜面反射：平面镜、球面镜、曲面镜均支持，可配置反射次数与衰减。
 *	 - 天空：基于 HSL 模型的动态程序化天空，颜色随时间缓慢变化。
 *	 - 投影：支持平面透视投影与球面鱼眼投影两种模式，实时切换。
 *
 *  2. 世界系统
 *	 - 分层体素存储 (LOD 0~6)：在保证精度的同时加速远距离遍历。
 *	 - 普通方块：6 个面各自拥有 16×16 调色板纹理。
 *	 - 特殊方块：可包含自定义三角形面、镜面球、球面纹理片等复杂几何。
 *	 - 方块变换：48 种正交旋转变换矩阵，支持方块整体旋转和面内自旋，
 *	   纹理与几何体会随变换正确映射。
 *
 *  3. 用户交互
 *	 - 键盘控制：
 *	   移动 —— W/S 前后，A/D 左右，P/L 上升/下降，T/G 水平前进/后退。
 *	   视角 —— Q/E 水平旋转，R/F 俯仰角度；鼠标拖拽也可控制视角。
 *	   编辑 —— B/V 放置/删除注视方块，X/Z 沿镜面反射方向放置/删除。
 *	   方块选择 —— 数字键 1~9 选择快捷栏方块，Tab 自定义快捷栏映射。
 *	   变换 —— N/M 旋转注视方块的朝向/自旋。
 *	   系统 —— H 切换彩色/字符模式，Y 切换球面投影，U 切换鼠标捕获，
 *			  -/= 调整渲染分辨率，[/] 调整视距。
 *	 - 鼠标控制：
 *	   移动鼠标控制视角朝向，左键删除方块，右键放置方块，
 *	   鼠标自动锁定在窗口中央，可随时通过 U 键释放/捕获。
 *
 *  4. 界面与输出
 *	 - 彩色模式：通过 ANSI 转义序列直接向控制台输出 24 位真彩色像素，
 *	   采用差量更新，只重绘变化的部分，大幅降低闪烁并提升帧率。
 *	 - 字符模式：用特殊符号表示面朝向和方块类型，生成老式字符画效果。
 *	 - 状态栏：固定于屏幕顶部，实时显示坐标、视角、FPS、当前方块、
 *	   注视方块信息等。
 *	 - 中心准星：彩色模式下在画面中央绘制白色十字准星。
 *
 *  5. 文件与存档
 *	 - 存档 (k 键)：将当前世界所有方块、旋转状态保存到 ./saves/ 目录下
 *	   的文本文件，文件名自动生成为当前日期时间。
 *	 - 读档 (j 键)：列表显示存档文件，支持预览缩略图，选择后加载。
 *	 - 截图 (c 键)：同时保存当前画面的字符版本（.txt）和彩色位图
 *	   （.bmp）到 ./images/ 目录，位图会进行垂直拉伸以便查看。
 *	 - 世界数据导出 (` 键)：将所有 LOD 层的方块数据导出为文本文件，
 *	   供调试或外部工具分析。
 *
 *  6. 资源与配置
 *	 - 方块定义文件存放于 ./texture/ 目录，由 all_world_cube.txt 索引，
 *	   普通方块纹理以 6×16×16 格式存储，特殊方块存储几何及纹理数据。
 *	 - 程序启动时自动生成一个简单的草地与石头构成的示例场景。
 *	 - 大部分行为参数通过常量宏定义在代码头部，方便调整：
 *	   渲染距离、反射次数、反射衰减、移动步长、鼠标灵敏度等。
 *
 * 【关键技术实现要点】
 *   - 光线追踪主循环：gogogo() 函数递归追踪，处理普通方块、镜面、
 *	 特殊方块的不同击中逻辑，并累计反射衰减。
 *   - 体素遍历：eye_next() 基于 DDA 思想计算射线到下一体素边界的距离，
 *	 结合 advance_ray() 实现逐体素步进。
 *   - 反射计算：标准反射向量公式 R = V - 2(N·V)N，支持多级递归。
 *   - 特殊方块几何：利用重心坐标进行纹理插值，对曲面则构造二次方程求解。
 *   - 浮点精度控制：round_self() 将向量四舍五入到 1e-8 精度，
 *	 结合 EPSILON 避免自交与精度错误。
 *   - 性能优化：彩色模式差量渲染、分辨率可调、多层 LOD 加速远距查询、
 *	 浮点运算舍入、编译优化 -O3 等。
 *
 * 【编译与运行环境】
 *   - 语言：C++17（部分库为 POSIX 扩展，使用 GCC/MinGW 编译）。
 *   - 操作系统：Windows 10 或更高（需控制台支持 ANSI 转义及 VT 处理）。
 *   - 编译示例：
 *	   g++ -static -o -std=c++17 -O3 […].exe […].cpp
 *   - 运行前需在程序同目录下创建 texture/ 目录并放入所需的纹理定义文件，
 *	 saves/ 和 images/ 目录会在首次保存时自动创建。
 *
 * 【控制速查表】
 *   移动：		  W/S 	   前后
 *				  A/D 	   左右
 *				  P/L 	   上升/下降
 *				  T/G 	   水平方向前进/后退（忽略俯仰）
 *   视角：		  Q/E 	   水平旋转
 *				  R/F 	   俯仰调整
 *				  鼠标		拖拽旋转视角
 *   编辑：		  B 		 放置当前快捷栏方块
 *				  V 		 删除注视方块
 *				  X 		 沿镜面反射方向放置方块
 *				  Z 		 沿镜面反射方向删除方块
 *   方块：		  1~9 	   选择快捷栏方块
 *				  Tab 	   自定义快捷栏映射
 *				  N/M 	   旋转注视方块的朝向/自旋
 *   系统：		  H 		 切换彩色/字符模式
 *				  Y 		 切换球面投影
 *				  U 		 切换鼠标捕获/释放
 *				  -/=		降低/提高渲染分辨率
 *				  [/]		减小/增大视距
 *   文件：		  C 		 保存当前画面截图
 *				  K 		 保存当前世界存档
 *				  J 		 浏览并加载存档
 *				  `		  导出世界数据
 *
 *   （更多详细说明与数学原理请参见下方“程序数学原理详解”注释块）
 * ============================================================================
 */


/*
 * ============================================================================
 * 程序数学原理详解（完整版）
 * ============================================================================
 *
 * 本程序以光线追踪为核心，结合体素遍历、多种几何体求交、反射、变换
 * 及程序化天空等模块，涉及大量数学知识。以下按功能模块分类阐述其
 * 底层数学原理。
 *
 * ============================================================================
 * 一、三维向量运算
 * ============================================================================
 *
 * 1. 向量表示
 *	- 结构体 node 封装三维向量 (x, y, z)。
 *	- 重载 +, -, *, /, +=, -= 等运算符，支持向量与标量的混合运算。
 *
 * 2. 基本运算
 *	- 长度：lnth(a) = sqrt(a.x2 + a.y2 + a.z2)
 *	- 点积：dot(a,b) = a.x*b.x + a.y*b.y + a.z*b.z
 *	  用于计算夹角余弦、投影、方向判别。
 *	- 叉积：cross(a,b) = (a.y*b.z - a.z*b.y,
 *						   a.z*b.x - a.x*b.z,
 *						   a.x*b.y - a.y*b.x)
 *	  用于求法向量、构建局部坐标系。
 *	- 归一化：normalize(a) = a / lnth(a)
 *	  得到单位方向向量，避免长度影响后续计算。
 *
 * 3. 反射向量
 *	- 公式：R = V - 2 (N·V) N
 *	  其中 V 为入射方向（指向交点），N 为交点处单位法向量（指向入射侧）。
 *	- 用于镜面反射方向更新，实现平面镜、球面镜和曲面镜效果。
 *
 * ============================================================================
 * 二、光线追踪核心：几何求交
 * ============================================================================
 *
 * 1. 轴对齐立方体（AABB）体素遍历（DDA 算法思想）
 *	函数：eye_next()、advance_ray()
 *	- 射线参数方程：P(t) = orig + t * dir
 *	- 对每个维度 i ∈ {x, y, z}，计算射线从当前位置到达该维度下一个
 *	  体素边界所需的 t 值：
 *	    if dir[i] > 0 : boundary = floor(pos[i] + 1)
 *					   （若存在多层LOD，按lod放大）
 *	    else 		 : boundary = ceil(pos[i] - 1)
 *	    t_i = (boundary - pos[i]) / dir[i]
 *	- 取最小的正 t 值对应轴作为首先穿过的面，完成一次步进。
 *	- 此算法实质是“A Fast Voxel Traversal Algorithm for Ray Tracing”
 *	  (Amanatides & Woo) 的一种实现，可高效地穿过连续体素空间。
 *
 * 2. 射线与任意三角形求交（M?ller–Trumbore 算法）
 *	函数：rayTriangleIntersect()
 *	- 给定三角形顶点 v0, v1, v2，边向量 edge1 = v1 - v0, edge2 = v2 - v0。
 *	- 计算 pvec = cross(dir, edge2), 行列式 det = dot(edge1, pvec)。
 *	- 若 |det| < ε，射线与三角形平行，无交点。
 *	- 令 invDet = 1/det, tvec = orig - v0。
 *	- 计算重心坐标 u = dot(tvec, pvec) * invDet，
 *		  若 u<0 或 u>1 则不相交。
 *	- 计算 qvec = cross(tvec, edge1), v = dot(dir, qvec) * invDet，
 *		  若 v<0 或 u+v>1 则不相交。
 *	- 交点参数 t = dot(edge2, qvec) * invDet，t 必须 ≥ 0（考虑 ε 偏移）。
 *	- 该算法无需预计算平面方程，计算量小，用于特殊方块中的三角形面片渲染。
 *
 * 3. 射线与球体求交
 *	函数：ball_is_hit()
 *	- 球面方程：||P - C||2 = r2
 *	- 射线：P = O + t * D（D 已归一化）
 *	- 代入得二次方程：a t2 + b t + c = 0
 *		 a = dot(D, D) = 1（已归一化）
 *		 b = 2 * dot(D, O - C)
 *		 c = dot(O - C, O - C) - r2
 *	- 判别式 Δ = b2 - 4ac，若 Δ < 0 无交点。
 *	- 两交点 t = (-b ± √Δ) / (2a)，取较小的正实数作为入射点。
 *	- 用于球面镜（mirror_round）的渲染，法向量为交点减球心再归一化。
 *
 * 4. 射线与双线性曲面片求交（带纹理的球面片）
 *	函数：part_round_to_mn()、part_is_hit()
 *	这是程序中最复杂的几何求交之一，用于渲染具有纹理坐标的球面局部曲面。
 *
 *	(1) 曲面定义
 *		- 球心 C，半径 r。
 *		- 球面上一个四边形区域由参数化方式定义：给定四个角点
 *		  L1, L2, L3, L4（均位于球面上且共面或近似共面），
 *		  以及射线方向方向向量 dir（即 L5，实际代码中 L5 是交点指向球心，详见下）。
 *		- 曲面上的点 P 可用双线性插值表示：
 *		  P(m,n) = L3 + A m + B n + C m n
 *		  其中 A = L2 - L3, B = L4 - L3, C = L1 - L2 - L4 + L3。
 *		  m, n ∈ [0,1] 为曲面参数。
 *
 *	(2) 转化为二维问题
 *		- 射线参数方程 P(t) = O + t * D，需同时满足 P(t) = 曲面上的点。
 *		- 为简化，代码中 L5（即函数参数中的 dir）实际是单位化的从交点指向球心的向量。 
 *		  其实在调用时，传入的 L5 是 tmpjd - pr.X，即从球心指向交点的向量，归一化后作为
 *		  方向建立局部坐标系。但函数中将其视为一个单位向量 dir，并构造正交基 (u, v)，
 *		  使得 u, v, dir 构成右手系。
 *		- 然后，将曲面上点相对于球心的向量表达在 (u, v) 坐标系中：
 *		  将 A, B, C 分别投影到 u, v 轴上，得到标量系数：
 *		   a1 = dot(A,u), b1 = dot(B,u), c1 = dot(C,u), d1 = -dot(L3,u)
 *		   a2 = dot(A,v), b2 = dot(B,v), c2 = dot(C,v), d2 = -dot(L3,v)
 *		- 由于曲面上的点 P 与射线方向 dir 共线（即 P - C 平行于 dir），
 *		  其在 u, v 方向上的坐标应为 0。因此我们要求 P 在 u, v 上的投影为零：
 *		   a1 m + b1 n + c1 m n + d1 = 0
 *		   a2 m + b2 n + c2 m n + d2 = 0
 *		- 这是一个关于 m, n 的二元二次方程组。
 *
 *	(3) 消元求解
 *		- 从第一个方程解出 m（或 n）并代入第二个方程，可得关于 n 的一元二次方程。
 *		  代码中通过构造关于 n 的二次方程：coeff_n2 * n2 + coeff_n1 * n + coeff_n0 = 0
 *		  系数计算方式如下（通过消去 m）：
 *		    coeff_n2 = b2*c1 - c2*b1
 *		    coeff_n1 = -b1*a2 + b2*a1 + c2*d1 - d2*c1
 *		    coeff_n0 = d1*a2 - d2*a1
 *		- 解该二次方程得到 n 的候选值，代入原二元一次关系求 m，
 *		  并只保留 m,n ∈ [0,1] 且满足与射线方向同向（dot(P-C, dir) > 0）的解。
 *		- 若存在有效 (m,n)，则击中曲面片，参数 t = ||P - O|| / ||D||（在调用处由球交计算 t）。
 *		  纹理坐标由 (m,n) 直接映射到 16×16 像素。
 *
 *	(4) 说明
 *		- 该数学处理本质上是在二维参数空间求解射线与双线性曲面的交点，
 *		  利用了“射线方向与曲面法向关系”投影到切平面进行降维。
 *		- 注意代码中的 L5 实际是交点处指向球心的反向向量，函数内部又归一化并构造 u,v，
 *		  实际相当于定义了以球心为原点、dir 为法向的一个切平面，曲面上的点在该切平面上
 *		  的投影需要满足与原点连线沿 dir 方向，从而转化为 m,n 的代数方程。
 *
 * 5. 交点选择与距离比较
 *	- 每次计算得到交点参数 t 后，与当前最小距离 dis 比较，若更小则更新最佳交点信息。
 *	- 保证了只保留相机（或反射点）最近的可见点。
 *
 * ============================================================================
 * 三、坐标变换与视角模型
 * ============================================================================
 *
 * 1. 球坐标转方向向量
 *	- 水平方位角 a（0°~360°），俯仰角 b（-90°~90°）。
 *	- 视线方向：
 *	    eye_dir.x = cos(b) * cos(a)
 *	    eye_dir.y = cos(b) * sin(a)
 *	    eye_dir.z = sin(b)
 *	- 水平方向（忽略俯仰）：
 *	    eye_xydir = (cos(a), sin(a), 0)
 *
 * 2. 屏幕空间基向量（右手系）
 *	- screen_x_dir：屏幕水平向右
 *	    screen_x_dir = (sin(a), -cos(a), 0)   （与 eye_dir、垂直轴正交）
 *	- screen_y_dir：屏幕垂直向上
 *	    screen_y_dir = (-sin(b)*cos(a), -sin(b)*sin(a), cos(b))
 *	- 这三者构成正交右手系，且 screen_y_dir 与 eye_dir 正交。
 *
 * 3. 像素射线生成
 *	- 平面屏幕模式 (is_round = false)：
 *		 eye_to_pixel = eye_act
 *					  + screen_x_dir * (screen_x_wid * (2*i/W - 1))
 *					  + screen_y_dir * (screen_y_wid * (2*j/H - 1))
 *		其中 eye_act = eye_dir * view_r，表示视点前方 r 处的屏幕中心。
 *		此为简单的透视投影，无镜头畸变。
 *	- 球形屏幕模式 (is_round = true)：
 *		令 θ_i = (i/W) * 360°，φ_j = (j/H) * 180°，
 *	    eye_to_pixel = eye_act * sin(θ_i+90°)*sin(φ_j+180°)
 *		实际代码为：
 *		  eye_act * (sin(...)*sin(...)) + screen_x_dir * (...) + screen_y_dir * (...)
 *		本质是把像素映射到单位球面上，产生鱼眼或球幕效果。
 *
 * ============================================================================
 * 四、纹理映射与插值
 * ============================================================================
 *
 * 1. 普通方块纹理
 *	- 击中面通过坐标分量与阈值（0或1）判断面索引。
 *	- 利用 refer 数组获取该面的两个纹理轴向，将击中点在该面内的坐标缩放到[0,1]，
 *	  然后乘以16取整，得到16×16纹理像素坐标。
 *
 * 2. 特殊方块三角形面片纹理
 *	- 四边形面片拆分为两个三角形，分别进行重心坐标计算。
 *	- 重心坐标 (u, v) 对每个三角形定义：
 *		三角形 (v0,v1,v2)：P = v0 + u*(v1-v0) + v*(v2-v0)
 *	- 转换为统一的纹理坐标 (s, t)：
 *		* 击中第一三角形 (0,1,2)：s = u+v, t = v
 *		* 击中第二三角形 (0,2,3)：s = u,   t = u+v
 *	- s, t ∈ [0,1]，映射到 16×16 纹理像素：uu = floor(s*16), vv = floor(t*16)
 *
 * 3. 曲面片纹理 (part_round)
 *	- 由 part_round_to_mn() 返回的参数 (m, n) 直接作为纹理坐标，
 *	  映射到 16×16 纹理：uu = floor(m*16), vv = floor(n*16)
 *
 * ============================================================================
 * 五、颜色科学与天空模型
 * ============================================================================
 *
 * 1. HSL 转 RGB
 *	函数 hsl_rgb(h, s, l)，h ∈ [0°,360°), s,l ∈ [0,1]
 *	- 计算中间量：
 *	    c = (1 - |2l - 1|) * s
 *	    x = c * (1 - |(h/60°) mod 2 - 1|)
 *	    m = l - c/2
 *	- 根据 h 所处扇区分配 (r',g',b')：
 *	    0~60: (c,x,0)   60~120: (x,c,0)  120~180: (0,c,x)
 *	    180~240: (0,x,c) 240~300: (x,0,c) 300~360: (c,0,x)
 *	- 最终 RGB = ((r'+m)*255, (g'+m)*255, (b'+m)*255)
 *
 * 2. 动态天空色
 *	函数 getSkyColor(direction)
 *	- 时间因子：timeFactor = sky_now_time / 60.0 (秒)
 *	- 垂直因子：verangleFactor = atan2(z, √(x2+y2)) / π + 0.5，范围 [0,1]
 *	- 水平因子：horangleFactor = atan2(x, y) / π，范围 [-1,1]
 *	- 映射到 HSL：
 *	    H = timeFactor * 360°
 *	    S = |horangleFactor|
 *	    L = verangleFactor
 *	- 此模型使天空随时间呈现色调循环，且上亮下暗，水平方向饱和度变化。
 *
 * ============================================================================
 * 六、数值计算与精度控制
 * ============================================================================
 *
 * 1. 浮点舍入
 *	- round_self(n): 各分量乘以 10^8 后四舍五入再除以 10^8，
 *	  有效抑制累积误差，用于方向向量、位置等关键数据。
 *	- 自定义三角函数 sinn/coss：输入度为角度，内部转为弧度并舍入到同样精度。
 *
 * 2. EPSILON 常量：1e-7
 *	- 用于浮点比较、避免除零、射线起点偏移、边界判定等。
 *
 * ============================================================================
 * 七、旋转变换与正交矩阵
 * ============================================================================
 *
 * 1. 方块旋转系统概述
 *	- 每个方块有 48 种不同的朝向，由 6 个面 × 8 个面内稳定子群构成。
 *	- 函数 rotate(nd, op, is_rev) 对块内坐标 [0,1]3 实施正交变换，
 *	  变换中心为 (0.5, 0.5, 0.5)，矩阵为 matrix_T 或其逆。
 *	- 矩阵来源：R_face[6] 将参考面 (z=1) 映射到 6 个方向；
 *	  S[8] 为保持 z=1 面不变的 8 个对称变换（4个旋转 + 4个反射）；
 *	  组合 T = R_face * S 得到全部 48 个正交矩阵。
 *	- 所有矩阵行列式绝对值 1，逆矩阵即转置，存储在 matrix_T_inv 中用于纹理采样。
 *
 * 2. 欧拉角视角旋转
 *	- 通过鼠标增量 dx, dy 调整 a（水平角）和 b（俯仰角），
 *	  死区 DEADZONE 避免微小抖动，然后重新计算方向向量。
 *
 * ============================================================================
 * 八、物理运动
 * ============================================================================
 *
 * 1. 玩家移动
 *	- position += direction * step
 *	- 支持沿视线方向 (W/S)、水平垂直方向 (P/L)、左右方向 (A/D)、
 *	  水平面前进后退 (T/G) 等。
 *
 * ============================================================================
 * 九、数据预处理与查找
 * ============================================================================
 *
 * 1. 反射衰减预计算
 *	- mr_pow[i] = mirror_reduction^i，用于快速计算多次反射后的颜色衰减。
 *
 * 2. 2的幂预计算
 *	- pow_2[0..10] 用于 LOD 缩放和存档跳页。
 *
 * ============================================================================
 * 注：以上数学原理与程序的具体实现细节高度耦合，详细实现请参见对应函数。
 * ============================================================================
 */





//color内部操作应为 point,输入输出才转化为ll 




//去掉无用的数字ll  ,  .0


//-lOpenCL -DUSE_OPENCL


#pragma GCC optimize(3,"Ofast","inline")
#include<bits/stdc++.h>

#ifdef USE_OPENCL
#include <CL/cl.hpp>
#endif

#include<windows.h>
#include<conio.h>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/time.h>
#define ll long long
#define M_PI 3.14159265358979323
#define EPSILON 1e-7
#define INF 1e18 
#define dirchar "?BFRLDU!"
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////结构体 

//结构体定义
#pragma pack(push, 1)
struct BMPFileHeader {
	uint16_t bfType = 0x4D42;	  // "BM"
	uint32_t bfSize = 0;
	uint16_t bfReserved1 = 0;
	uint16_t bfReserved2 = 0;
	uint32_t bfOffBits = 54;		// 24位BMP文件头+信息头共54字节
};

struct BMPInfoHeader {
	uint32_t biSize = 40;
	int32_t  biWidth = 0;
	int32_t  biHeight = 0;		   // 正数表示底部→顶部存储
	uint16_t biPlanes = 1;
	uint16_t biBitCount = 24;
	uint32_t biCompression = 0;	  // BI_RGB
	uint32_t biSizeImage = 0;
	int32_t  biXPelsPerMeter = 0;
	int32_t  biYPelsPerMeter = 0;
	uint32_t biClrUsed = 0;
	uint32_t biClrImportant = 0;
};
#pragma pack(pop)

struct point;
struct node {
    double x,y,z;
    node(double x=0,double y=0,double z=0):x(x),y(y),z(z){}

    bool operator<(const node &aa) const { return std::tie(x, y, z) < std::tie(aa.x, aa.y, aa.z); }
    bool operator==(const node &aa) const { return (x == aa.x) && (y == aa.y) && (z == aa.z); }
    bool operator!=(const node &aa) const { return !(*this == aa); }

    node operator+(const node &aa) const { return {x + aa.x, y + aa.y, z + aa.z}; }
    node operator-(const node &aa) const { return {x - aa.x, y - aa.y, z - aa.z}; }
    node operator*(const double &aa) const { return {x * aa, y * aa, z * aa}; }
    node operator/(const double &aa) const { return {x / aa, y / aa, z / aa}; }

    node &operator+=(const node &aa) { x += aa.x; y += aa.y; z += aa.z; return *this; }
    node &operator-=(const node &aa) { x -= aa.x; y -= aa.y; z -= aa.z; return *this; }
    node &operator*=(const double &aa) { x *= aa; y *= aa; z *= aa; return *this; }
    node &operator/=(const double &aa) { x /= aa; y /= aa; z /= aa; return *this; }

    // 与 point 的混合运算
    node operator+(const point &aa) const;
    node operator-(const point &aa) const;
    node &operator+=(const point &aa);
    node &operator-=(const point &aa);

    friend double lnth(const node &aa) { return std::sqrt(aa.x * aa.x + aa.y * aa.y + aa.z * aa.z); }
    friend node cross(const node &aa, const node &bb) {
        return {aa.y * bb.z - aa.z * bb.y,
                aa.z * bb.x - aa.x * bb.z,
                aa.x * bb.y - aa.y * bb.x};
    }
    friend double dot(const node &aa, const node &bb) { return aa.x * bb.x + aa.y * bb.y + aa.z * bb.z; }
    friend node normalize(const node &aa) { return aa / lnth(aa); }
    friend double manhattan_dis(const node &aa, const node &bb){return fabs(aa.x-bb.x)+fabs(aa.y-bb.y)+fabs(aa.z-bb.z);}
};

struct point{
    ll x,y,z;
    point(ll x=0,ll y=0,ll z=0):x(x),y(y),z(z){}
	
    bool operator<(const point &aa) const { return std::tie(x, y, z) < std::tie(aa.x, aa.y, aa.z); }
    bool operator==(const point &aa) const { return (x == aa.x) && (y == aa.y) && (z == aa.z); }
    bool operator!=(const point &aa) const { return !(*this == aa); }

    // point 与 point
    point operator+(const point &aa) const { return {x + aa.x, y + aa.y, z + aa.z}; }
    point operator-(const point &aa) const { return {x - aa.x, y - aa.y, z - aa.z}; }
    point &operator+=(const point &aa) { x += aa.x; y += aa.y; z += aa.z; return *this; }
    point &operator-=(const point &aa) { x -= aa.x; y -= aa.y; z -= aa.z; return *this; }

    // point 与 node（结果为 node）
    node operator+(const node &aa) const { return {x + aa.x, y + aa.y, z + aa.z}; }
    node operator-(const node &aa) const { return {x - aa.x, y - aa.y, z - aa.z}; }

    // point 与 double（结果提升为 node）
    node operator*(const double &aa) const { return {x * aa, y * aa, z * aa}; }
    node operator/(const double &aa) const { return {x / aa, y / aa, z / aa}; }

    // point 与 ll（保持整数）
    point operator*(const ll &aa) const { return {x * aa, y * aa, z * aa}; }
    point &operator*=(const ll &aa) { x *= aa; y *= aa; z *= aa; return *this; }
	
    friend ll manhattan_dis(const point &aa, const point &bb){return llabs(aa.x-bb.x)+llabs(aa.y-bb.y)+llabs(aa.z-bb.z);}
};

// node 与 point（需要 point 的完整定义）
inline node node::operator+(const point &aa) const { return {x + aa.x, y + aa.y, z + aa.z}; }
inline node node::operator-(const point &aa) const { return {x - aa.x, y - aa.y, z - aa.z}; }
inline node &node::operator+=(const point &aa) { x += aa.x; y += aa.y; z += aa.z; return *this; }
inline node &node::operator-=(const point &aa) { x -= aa.x; y -= aa.y; z -= aa.z; return *this; }

struct cube_face{//special cube面定义 
	node X[4];
	vector<vector<point> >color=vector<vector<point> >(16,vector<point>(16,0));
};
struct mirror_round{//special cube球镜定义 
	node X;
	double r=0;
};
struct part_round{//special cube球片定义 
	node X;
	double r=0;
	node L[4];
	vector<vector<point> >color=vector<vector<point> >(16,vector<point>(16,0));
};
struct special_cube{//特殊方块 
	ll cube_face_num=0;
	vector<cube_face>_cube_face;
	ll mirror_round_num=0;
	vector<mirror_round>_mirror_round; 
	ll part_round_num=0;
	vector<part_round>_part_round;
};
struct hit_cube{
	//输入 
	point cube_num;
	node v={1,0,0};
	node p0={0,0,0};
	
	
	//输出 
	bool is_hit=0;
	
	bool is_mirror=0;
	
	point res_color=0;
	char ch='?';
	ll inlight_direction=0;
	
	
	node jd={0,0,0};
	
	node n={1,0,0};
	node v_next={1,0,0};
	
	node p={0,0,0};
};
//struct cube_state{
//	ll cube;
//	ll rot_state;
//};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////（半）常量 

//常量 
const point zzz={0,0,0};

const ll refer[3][2]={{1,2},{0,2},{0,1}};//xyz互补对应关系 
const ll pow_2[11]={1,2,4,8,16,32,64,128,256,512,1024};//2的幂 

const ll mirror_times=25;//镜子反射次数 
const double mirror_reduction=0.8;

const ll tens=100000000;

const ll resolution_x[10]={0,64,128,192,256,384,512,1024,1280};
const ll resolution_y[10]={0,18,36,54,72,108,144,288,360};
const double view_r_opt[20]={0,0.01,0.02,0.05,0.1,0.2,0.5,1,2,4,8,16};
const double screen_x_wid=0.4,screen_y_wid=0.225;

const ll updown_degree=2,leftright_degree=2;
const double forbackward_step=0.2,leftright_step=0.2;

const ll render_distance=32;

const double mouse_sensitivity=0.1;
const ll DEADZONE=1;

//魔法数字
const double MIRROR_CENTER=7.0/16;

const double CUBE_CENTER=0.5;
const node NODE_CUBE_CENTER={CUBE_CENTER,CUBE_CENTER,CUBE_CENTER};

const double INVALID_D=-1.0;
const node NODE_INVALID_D={INVALID_D,INVALID_D,INVALID_D};
const pair<double,double> PDD_INVALID_D={INVALID_D,INVALID_D};

const ll INVALID_LL=-1;
const point POINT_INVALID_LL={INVALID_LL,INVALID_LL,INVALID_LL};

const ll TEX_SIZE=16;

const ll INFO_TIMEOUT=120000000;

const ll CHAR_MAXNUM=1024;

const double SECOND_TO_MICROSECOND=1e6;

const ll MAX_LOD=4; 
const ll VERTEX_NUM=8;
const ll FACE_NUM=6;
const ll CUBE_SELF=7;

const ll INVISIBLE=-1;
const ll REFLECTIVE=-2;
const point INV={INVISIBLE,INVISIBLE,INVISIBLE};
const point REF={REFLECTIVE,REFLECTIVE,REFLECTIVE};
enum SPECIAL_CUBE{
	CUBE_fACE=1,MIRROR_ROUND,PART_ROUND
};
enum ORDINAL{
	UNKNOWN,FIRST,SECOND,THIRD
};
//半常量
vector<string>idcube;
vector<bool>is_special(0);

double matrix_T[48][3][3];
double matrix_T_inv[48][3][3];
double mr_pow[60];

ll kind_of_cube=0;
vector<vector<vector<vector<point> > > >all_cube_color;
map<string,ll>rev_idcube;//方块名称对应id 
vector<special_cube>sp_cube;
vector<vector<special_cube> >rot_sp_cube;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////变量 

//bool 状态变量 
bool first_frame=true;
bool force_redraw=false;
bool is_mouse=false;
bool change=false;
bool is_color=false;
bool is_round=false;

//ll 状态变量
ll view_r_state=7;
ll now_cube=1;
ll resolution=3;

//图像变量
string prev_image_s;
string image_s;
vector<point>prev_screen_color;
string ansi_buffer;
vector<point>current_frame;

//时间变量
vector<double>recent_FPS;
double now_FPS=0;
double sky_now_time;

//鼠标变量
ll lastX=0,lastY=0;
HWND g_hConsoleWnd=NULL;
bool lastLeft=false,lastRight=false; 
bool leftDown,rightDown;

//键盘变量
ll key_ch; 

//视角与位置变量 
double view_r=1;
node position={-1,0,0},eye_dir,eye_act,screen_x_dir,screen_y_dir,eye_xydir;
double view_a=0,view_b=0;

//信息变量 
string info="";
ll info_time=0;

//快捷键对应
ll shortcut_key[10]={0,1,2,3,4,5,6,7,8,9}; 
ll cube_to_shortcut[30]={0,1,2,3,4,5,6,7,8,9};

//世界变量 
map<point,ll>cube[10];//cube[0]表示对应位置方块的编号；cube[1~MAX_LOD]表示对应"区块"的方块个数 
map<point,ll>rot_state;//旋转翻折编号



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////所有定义的函数 

//工具函数
node round_self(node n);
double sinn(double x);
double coss(double x);
point to_point(node x);
node to_node(point x);
point color_ll_to_point(ll color);
ll color_point_to_ll(point color);
ll choose_which_face(node n,node direction);
ll select_axis(node t);
string downsample(const string&input,ll target_rows,ll target_cols);
node rotate(node nd,ll op,bool is_rev);
special_cube cube_rot(special_cube cube_num,ll op);
ll generate_block(point pos);
ll generate_voxel(ll lod,point pos);
ll get_block(ll lod,const point&pos);
ll get_rotate(const point&pos);
void save_frame_as_bmp(const string& filename,int32_t width,int32_t height,const vector<point>& frame_data);

//系统函数
void HideCursor();
void CaptureMouseMovement(ll& dx,ll& dy);
void gotoxy(ll x,ll y);
void ScrollWindowToCursor();
bool EnableVirtualTerminal();
ll getTimeOfDayMicros();
ll get_nowtime();
vector<string>getSaveFileNames();
void mouse();
void keyboard();

//初始化函数
void init_transform_matrices();
void over_all_init();
void init();

//找颜色函数 
point find_color(point cube_num,node cube_relative);
point hsl_rgb(double h,double s,double l);
point getSkyColor(node direction);

//一像素填充函数 
char find_char(node cube_relative);
ll find_face(node cube_relative);
void record_s(ll cube_id,ll inlight_direction);
void record_s_by_char(char ch);
void print(point color,ll mr_times,ll pixel_idx);

//体素计算函数
node eye_next(node cur_position,node direction);
void advance_ray(ll axis,node dir,node t_dist,node& pos,point& cube_num);

//特殊方块判断函数
bool rayTriangleIntersect(const node& orig, const node& dir,const node& v0, const node& v1, const node& v2,double& t, double& outU, double& outV);
pair<double,double>part_round_to_mn(const node&L1,const node&L2,const node&L3,const node&L4,const node&L5);
void face_is_hit(cube_face cf,node p0,node v,cube_face&best_cf,double&dis,node&res_color,node&best_jd,ll&which_special);
void ball_is_hit(mirror_round mr,node p0,node v,mirror_round&best_mr,double&dis,node&res_color,node&best_jd,ll&which_special);
void part_is_hit(part_round pr,node p0,node v,part_round&best_pr,double&dis,node&res_color,node&best_jd,ll&which_special);
bool special_is_hit(point cube_num,node cube_relative,node direction,bool& is_mirror,node& n,node& jd,point&res_color);

//3个gogogo函数
void calc_ray(hit_cube&hc);
void gogogo(node tmp_position,node direction,ll mr_times,ll pixel_idx);
pair<ll,point>eyegogogo(node direction);
pair<ll,point>mirror_eyegogogo(node tmp_position,node direction,ll mr_times);

//世界更改函数 
void add_cube(pair<ll,point>pid,ll put_cube);
void combo_delete_temp(point cube_num);
void delete_cube(pair<ll,point>pid);

//快捷键函数
void display_shortcut(ll pos);

//通知添加函数
void add_info(string message);

//信息添加函数
string format_status_line();
void add_center_cross();

//渲染函数
void calculate_whole();
void append_color_run(ll x,ll y,ll length,point color);
void optimized_render();

//保存函数
void touch_c(ll nt);
void touch_k(ll nt);

//读存档函数
string getPreviewImage(const string& filename);
void column_print(const string&content,ll column);
void displaySaveList(const vector<string>& fileNames,ll pos);
void init_chunk(ll lod,point pos);
void loadSave(const string& filename);

//区块储存函数
//void delete_lod(ll lod,node pos);
//void load_chunk(node pos);
//void save_chunk(node pos);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////gpu





#ifdef USE_OPENCL
class GPUAccelerator {
public:
    bool available = false;
    GPUAccelerator();
    void init();
    void uploadWorld(
	    const map<point, long long> (&cube)[10],
	    const map<point, long long>& rot_state,
	    const vector<vector<vector<vector<point>>>>& all_cube_color,
	    const double matrix_T_inv[48][3][3],
	    const vector<vector<special_cube>>& rot_sp_cube,
	    const node& position,
	    long long render_distance);
    void dispatchRender(
	    const node& eye_dir, const node& screen_x_dir, const node& screen_y_dir,
	    const node& eye_act, const node& eye_xydir,
	    double view_a, double view_b, double view_r,
	    bool is_round, long long res_x, long long res_y,
	    long long mirror_times, double mirror_reduction,
	    std::vector<point>& current_frame);
private:
    cl::Context context;
    cl::CommandQueue queue;
    cl::Kernel kernel_trace;
    cl::Device device; 
    cl::Buffer d_voxelGrid, d_textures, d_matrices;
    int gridSizeX, gridSizeY, gridSizeZ;
    node gridOrigin;
    float voxelScale = 1.0f;
};



// -------- 内嵌 OpenCL 内核源码字符串 --------
static const char* raytraceKernelSource = R"(
typedef struct {
    float x, y, z;
} float3_t;

typedef struct {
    int x, y, z;
} int3_t;

float3_t make_float3(float x, float y, float z) {
    float3_t v; v.x = x; v.y = y; v.z = z; return v;
}

float dot(float3_t a, float3_t b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
float3_t cross(float3_t a, float3_t b) {
    return make_float3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}
float3_t normalize3(float3_t v) {
    float len = sqrt(dot(v,v));
    return (len > 1e-7f) ? make_float3(v.x/len, v.y/len, v.z/len) : v;
}

// 向量运算函数（替代运算符）
float3_t add3(float3_t a, float3_t b) { return make_float3(a.x+b.x, a.y+b.y, a.z+b.z); }
float3_t sub3(float3_t a, float3_t b) { return make_float3(a.x-b.x, a.y-b.y, a.z-b.z); }
float3_t mul3(float3_t a, float s)    { return make_float3(a.x*s, a.y*s, a.z*s); }
float3_t mul3v(float3_t a, float3_t b){ return make_float3(a.x*b.x, a.y*b.y, a.z*b.z); }  // 逐分量乘
float3_t div3(float3_t a, float s)    { return make_float3(a.x/s, a.y/s, a.z/s); }
float3_t neg3(float3_t a)             { return make_float3(-a.x, -a.y, -a.z); }

int sampleVoxel(__global const int* grid,
                int gsx, int gsy, int gsz,
                int x, int y, int z) {
    if (x < 0 || x >= gsx || y < 0 || y >= gsy || z < 0 || z >= gsz)
        return 0;
    return grid[z * (gsy * gsx) + y * gsx + x];
}

unsigned int sampleTexture(__global const unsigned int* textures,
                           int cubeId, int face, int uu, int vv) {
    int texOff = ((cubeId * 6) + (face - 1)) * 256 + vv * 16 + uu;
    return textures[texOff];
}

float3_t rotateLocal(float3_t p, int rotState, __global const float* matrices) {
    float3_t c = make_float3(0.5f, 0.5f, 0.5f);
    float3_t q = sub3(p, c);
    int matOff = rotState * 9;
    float3_t r;
    r.x = matrices[matOff+0]*q.x + matrices[matOff+1]*q.y + matrices[matOff+2]*q.z;
    r.y = matrices[matOff+3]*q.x + matrices[matOff+4]*q.y + matrices[matOff+5]*q.z;
    r.z = matrices[matOff+6]*q.x + matrices[matOff+7]*q.y + matrices[matOff+8]*q.z;
    return add3(r, c);
}

int findFace(float3_t jd) {
    if (fabs(jd.x) < 1e-4f) return 1;
    if (fabs(jd.x - 1.0f) < 1e-4f) return 2;
    if (fabs(jd.y) < 1e-4f) return 3;
    if (fabs(jd.y - 1.0f) < 1e-4f) return 4;
    if (fabs(jd.z) < 1e-4f) return 5;
    if (fabs(jd.z - 1.0f) < 1e-4f) return 6;
    return 0;
}
int findFaceByNormal(float3_t n, float3_t jd) {
    float absN[3] = {fabs(n.x), fabs(n.y), fabs(n.z)};
    int axis = 0;
    if (absN[1] > absN[0]) axis = 1;
    if (absN[2] > absN[axis]) axis = 2;
    int sign = (axis==0) ? (n.x > 0) : ((axis==1) ? (n.y > 0) : (n.z > 0));
    return axis*2 + (sign ? 1 : 2);
}

void getTexCoords(float3_t jdLocal, int face, int* uu, int* vv) {
    float u, v;
    if (face == 1 || face == 2) {
        u = jdLocal.y; v = jdLocal.z;
    } else if (face == 3 || face == 4) {
        u = jdLocal.x; v = jdLocal.z;
    } else {
        u = jdLocal.x; v = jdLocal.y;
    }
    *uu = (int)(u * 16.0f); *vv = (int)(v * 16.0f);
    *uu = clamp(*uu, 0, 15); *vv = clamp(*vv, 0, 15);
}

unsigned int traceRay(float3_t origin, float3_t dir,
                      __global const int* grid, int gsx, int gsy, int gsz,
                      float3_t gridOrigin, float voxScale,
                      __global const unsigned int* textures,
                      __global const float* matrices,
                      int maxSteps, int maxBounce) {
    // 世界空间光线追踪，体素大小为 1（忽略 voxScale 因为当前始终为1）
    float3_t pos = origin;
    float3_t rayDir = dir;
    float3_t color = make_float3(0.0f, 0.0f, 0.0f);
    float3_t attenuation = make_float3(1.0f, 1.0f, 1.0f);
    int bounce = 0;

    while (bounce <= maxBounce) {
        // 转为相对于 gridOrigin 的坐标（世界单位）
        float3_t rpos = sub3(pos, gridOrigin);
        int3_t vox = { (int)floor(rpos.x), (int)floor(rpos.y), (int)floor(rpos.z) };

        // 出界 → 天空
        if (vox.x < 0 || vox.x >= gsx || vox.y < 0 || vox.y >= gsy || vox.z < 0 || vox.z >= gsz) {
            float upness = rayDir.z * 0.5f + 0.5f;
            float3_t skyColor = make_float3(0.5f + 0.5f*upness, 0.6f + 0.3f*upness, 0.8f + 0.2f*upness);
            color = add3(color, mul3v(attenuation, skyColor));
            break;
        }

        // ---------- DDA 初始化（世界空间） ----------
        float3_t tMax;
        float3_t step;
        int3_t stepSign;

        if (rayDir.x > 0) { step.x = 1;  tMax.x = (vox.x + 1 - rpos.x) / rayDir.x; stepSign.x = 1; }
        else              { step.x = -1; tMax.x = (rpos.x - vox.x)     / -rayDir.x; stepSign.x = -1; }
        if (rayDir.y > 0) { step.y = 1;  tMax.y = (vox.y + 1 - rpos.y) / rayDir.y; stepSign.y = 1; }
        else              { step.y = -1; tMax.y = (rpos.y - vox.y)     / -rayDir.y; stepSign.y = -1; }
        if (rayDir.z > 0) { step.z = 1;  tMax.z = (vox.z + 1 - rpos.z) / rayDir.z; stepSign.z = 1; }
        else              { step.z = -1; tMax.z = (rpos.z - vox.z)     / -rayDir.z; stepSign.z = -1; }

        bool hit = false;
        float3_t hitPos, hitNormal;
        int hitBlock = 0;
        int steps = 0;
        float tEnter = 0.0f;

        // DDA 步进
        while (steps < maxSteps) {
            // 选择最先击中的轴
            float tNext = tMax.x;
            int axis = 0;
            if (tMax.y < tNext) { tNext = tMax.y; axis = 1; }
            if (tMax.z < tNext) { tNext = tMax.z; axis = 2; }

            tEnter = tNext;     // 记录进入下一个体素的时间
            // 步进体素索引
            if (axis == 0) { vox.x += stepSign.x; tMax.x += 1.0f / fabs(rayDir.x); }
            else if (axis == 1) { vox.y += stepSign.y; tMax.y += 1.0f / fabs(rayDir.y); }
            else { vox.z += stepSign.z; tMax.z += 1.0f / fabs(rayDir.z); }

            // 检查是否离开网格
            if (vox.x < 0 || vox.x >= gsx || vox.y < 0 || vox.y >= gsy || vox.z < 0 || vox.z >= gsz) break;

            // 查找该体素是否有方块
            int block = sampleVoxel(grid, gsx, gsy, gsz, vox.x, vox.y, vox.z);
            if (block != 0) {
                hit = true;
                hitBlock = block;
                // 交点 = 射线起点 + 方向 * 进入时间
                hitPos = add3(origin, mul3(rayDir, tEnter));
                // 法线（指向射线来的方向）
                if (axis == 0) hitNormal = make_float3(stepSign.x > 0 ? -1.0f : 1.0f, 0, 0);
                else if (axis == 1) hitNormal = make_float3(0, stepSign.y > 0 ? -1.0f : 1.0f, 0);
                else hitNormal = make_float3(0, 0, stepSign.z > 0 ? -1.0f : 1.0f);
                break;
            }
            steps++;
        }

        if (!hit) {
            float upness = rayDir.z * 0.5f + 0.5f;
            float3_t skyColor = make_float3(0.5f + 0.5f*upness, 0.6f + 0.3f*upness, 0.8f + 0.2f*upness);
            color = add3(color, mul3v(attenuation, skyColor));
            break;
        }

        int blockId = hitBlock & 0xFFFF;
        int rotState = (hitBlock >> 16) & 0xFF;

        // ----- 普通方块纹理采样（暂时关闭镜面） -----
        // 计算体素最小角点的世界坐标
        float3_t worldVoxCorner = add3(gridOrigin, make_float3((float)vox.x, (float)vox.y, (float)vox.z));
        // 局部坐标 [0,1]^3，交点位于表面
        float3_t localJd = sub3(hitPos, worldVoxCorner);   // 各分量在 0~1 之间

        // 1. 根据未旋转的局部坐标判断面（CPU 做法）
        int face = 0;
        if (fabs(localJd.x) < 1e-4f) face = 1;
        else if (fabs(localJd.x - 1.0f) < 1e-4f) face = 2;
        else if (fabs(localJd.y) < 1e-4f) face = 3;
        else if (fabs(localJd.y - 1.0f) < 1e-4f) face = 4;
        else if (fabs(localJd.z) < 1e-4f) face = 5;
        else if (fabs(localJd.z - 1.0f) < 1e-4f) face = 6;

        // 如果精度不足，使用法线兜底
        if (face == 0) {
            if (fabs(hitNormal.x) > 0.5f) face = (hitNormal.x > 0) ? 2 : 1;
            else if (fabs(hitNormal.y) > 0.5f) face = (hitNormal.y > 0) ? 4 : 3;
            else face = (hitNormal.z > 0) ? 6 : 5;
        }

        // 2. 应用逆旋转，得到纹理坐标对应的局部坐标
        float3_t texLocal = rotateLocal(localJd, rotState, matrices);

        // 3. 采样纹理（面使用未旋转时判定的面，纹理坐标使用旋转后的）
        int uu, vv;
        getTexCoords(texLocal, face, &uu, &vv);
        unsigned int texCol = sampleTexture(textures, blockId, face, uu, vv);
        float3_t surfColor = make_float3((float)((texCol>>16)&0xFF)/255.0f,
                                         (float)((texCol>>8)&0xFF)/255.0f,
                                         (float)(texCol&0xFF)/255.0f);
        color = add3(color, mul3v(attenuation, surfColor));
        break;  // 无反射
    }

    int r = (int)(color.x * 255.0f);
    int g = (int)(color.y * 255.0f);
    int b = (int)(color.z * 255.0f);
    r = clamp(r, 0, 255); g = clamp(g, 0, 255); b = clamp(b, 0, 255);
    return (r << 16) | (g << 8) | b;
}

__kernel void raytrace_kernel(
    __global unsigned int* output,
    int width, int height,
    __global const int* voxelGrid,
    int gsx, int gsy, int gsz,
    float gridOriginX, float gridOriginY, float gridOriginZ,
    float voxScale,
    __global const unsigned int* textures,
    __global const float* matrices,
    float posX, float posY, float posZ,
    float eyeDirX, float eyeDirY, float eyeDirZ,
    float screenXDirX, float screenXDirY, float screenXDirZ,
    float screenYDirX, float screenYDirY, float screenYDirZ,
    float screenXWid, float screenYWid,
    int maxSteps, int maxBounce
) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    if (x >= width || y >= height) return;

    float u = (x / (float)(width - 1)) * 2.0f - 1.0f;
    float v = (y / (float)(height - 1)) * 2.0f - 1.0f;

    float3_t eyePos = make_float3(posX, posY, posZ);
    float3_t eyeDir = make_float3(eyeDirX, eyeDirY, eyeDirZ);
    float3_t scrX = make_float3(screenXDirX, screenXDirY, screenXDirZ);
    float3_t scrY = make_float3(screenYDirX, screenYDirY, screenYDirZ);
    
    float3_t rayDir = add3(eyeDir, add3(mul3(scrX, u * screenXWid), mul3(scrY, v * screenYWid)));
    rayDir = normalize3(rayDir);

    unsigned int color = traceRay(eyePos, rayDir, voxelGrid, gsx, gsy, gsz,
                                  make_float3(gridOriginX, gridOriginY, gridOriginZ), voxScale,
                                  textures, matrices, maxSteps, maxBounce);
    output[y * width + x] = color;
}
)";


//static const char* raytraceKernelSource = R"(
//__kernel void test_kernel(__global uint* output) {
//    int id = get_global_id(0);
//    output[id] = id;
//}
//)";

// -------- 成员函数实现 --------
GPUAccelerator::GPUAccelerator(){}
void GPUAccelerator::init() {
    available = false;

    // 1. 获取平台
    std::vector<cl::Platform> platforms;
    cl_int err = cl::Platform::get(&platforms);
    if (err != CL_SUCCESS) {
        add_info("OpenCL: cl::Platform::get failed");
        return;
    }
    if (platforms.empty()) {
        add_info("OpenCL: No platforms found");
        return;
    }

    // 2. 寻找 GPU 设备
    cl::Platform chosen;
    bool gpuFound = false;
    for (auto& p : platforms) {
        std::vector<cl::Device> devs;
        err = p.getDevices(CL_DEVICE_TYPE_GPU, &devs);
        if (err == CL_SUCCESS && !devs.empty()) {
            chosen = p;
            device = devs[0];
            gpuFound = true;
            break;
        }
    }
    if (!gpuFound) {
        add_info("OpenCL: No GPU device found");
        // 可选：尝试 CPU 设备
        for (auto& p : platforms) {
            std::vector<cl::Device> devs;
            err = p.getDevices(CL_DEVICE_TYPE_CPU, &devs);
            if (err == CL_SUCCESS && !devs.empty()) {
                device = devs[0];
                add_info("OpenCL: Using CPU device instead");
                break;
            }
        }
        if (device() == nullptr) {
            add_info("OpenCL: No device (GPU or CPU) found");
            return;
        }
    }

    // 3. 创建上下文
    cl_int ctxErr = 0;
    context = cl::Context(device, nullptr, nullptr, nullptr, &ctxErr);
    if (ctxErr != CL_SUCCESS) {
        add_info("OpenCL: Context creation failed");
        return;
    }

    // 4. 创建命令队列
    cl_int queueErr = 0;
    queue = cl::CommandQueue(context, device, 0, &queueErr);
    if (queueErr != CL_SUCCESS) {
        add_info("OpenCL: CommandQueue creation failed");
        return;
    }

    // 5. 编译内核
    cl::Program::Sources sources;
    sources.push_back({raytraceKernelSource, strlen(raytraceKernelSource)});
    cl::Program program(context, sources, &err);
    if (err != CL_SUCCESS) {
        add_info("OpenCL: Program creation failed");
        return;
    }

    std::vector<cl::Device> buildDevices = {device};
    err = program.build(buildDevices);
    if (err != CL_SUCCESS) {
        // 获取编译日志
        std::string log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
        add_info("OpenCL: Build failed - " + log.substr(0, 100)); // 截取前100字符
        return;
    }

	kernel_trace = cl::Kernel(program, "raytrace_kernel", &err);
//	kernel_trace = cl::Kernel(program, "test_kernel", &err);
    if (err != CL_SUCCESS) {
        add_info("OpenCL: Kernel extraction failed");
        return;
    }

    available = true;
    add_info("OpenCL: Initialization successful");
}
void GPUAccelerator::uploadWorld(
    const map<point, long long> (&cube)[10],
    const map<point, long long>& rot_state,
    const vector<vector<vector<vector<point>>>>& all_cube_color,
    const double matrix_T_inv[48][3][3],
    const vector<vector<special_cube>>& rot_sp_cube,
    const node& position,
    long long render_distance)
{
    if (!available) return;
    // 1. 计算网格范围
    long long R = render_distance + 2;
    long long cx = (long long)floor(position.x);
    long long cy = (long long)floor(position.y);
    long long cz = (long long)floor(position.z);
    gridSizeX = 2 * R + 1;
    gridSizeY = 2 * R + 1;
    gridSizeZ = 2 * R + 1;
    gridOrigin = node(cx - R, cy - R, cz - R);

    // 2. 构建体素数据
    vector<int> voxelData(gridSizeX * gridSizeY * gridSizeZ, 0);
    for (int z = 0; z < gridSizeZ; ++z)
        for (int y = 0; y < gridSizeY; ++y)
            for (int x = 0; x < gridSizeX; ++x) {
                point wp(cx - R + x, cy - R + y, cz - R + z);
                long long block = 0;
                auto it = cube[0].find(wp);
                if (it != cube[0].end()) block = it->second;
                else block = generate_block(wp); // 可访问，因为在同一文件
                if (block > 0) {
                    long long rot = 0;
                    auto rit = rot_state.find(wp);
                    if (rit != rot_state.end()) rot = rit->second;
                    voxelData[z * (gridSizeY * gridSizeX) + y * gridSizeX + x] =
                        (int)(block & 0xFFFF) | ((int)(rot & 0xFF) << 16);
                }
            }
    d_voxelGrid = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                             sizeof(int) * voxelData.size(), voxelData.data());

    // 3. 上传纹理数据
	size_t numBlocks = all_cube_color.size() - 1;   // kind_of_cube
	std::vector<unsigned int> texData;
	// ---- 关键修复：填充方块0的纹理（6面 × 256像素），防止索引偏移 ----
	for (int i = 0; i < 6 * 256; ++i) {
	    texData.push_back(0);   // 全黑，空气不可见
	}
	for (size_t id = 1; id <= numBlocks; ++id) {
	    for (int face = 1; face <= 6; ++face) {
	        for (int v = 0; v < 16; ++v) {
	            for (int u = 0; u < 16; ++u) {
	                point col = all_cube_color[id][face][v][u];
	                unsigned int c = ((unsigned int)(col.x & 0xFF) << 16) | ((unsigned int)(col.y & 0xFF) << 8) | (unsigned int)(col.z & 0xFF);
	                texData.push_back(c);
	            }
	        }
	    }
	}
	d_textures = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,sizeof(unsigned int) * texData.size(), texData.data());

    // 4. 上传旋转逆矩阵
    std::vector<float> matData(48 * 9);
    for (int i = 0; i < 48; ++i) {
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                matData[i * 9 + r * 3 + c] = (float)matrix_T_inv[i][r][c];
            }
        }
    }
    d_matrices = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                            sizeof(float) * matData.size(), matData.data());

    // 5. 特殊方块几何数据暂未上传，后续可扩展
}
void GPUAccelerator::dispatchRender(
    const node& eye_dir, const node& screen_x_dir, const node& screen_y_dir,
    const node& eye_act, const node& eye_xydir,
    double view_a, double view_b, double view_r,
    bool is_round, long long res_x, long long res_y,
    long long mirror_times, double mirror_reduction,
    std::vector<point>& current_frame) {
    if (!available) return;

    int width = (int)(res_x + 1);
    int height = (int)(res_y + 1);
    size_t totalPixels = width * height;

    // 创建输出缓冲区
    cl::Buffer d_output(context, CL_MEM_WRITE_ONLY, sizeof(unsigned int) * totalPixels);

    // 设置内核参数
    kernel_trace.setArg(0, d_output);
    kernel_trace.setArg(1, width);
    kernel_trace.setArg(2, height);
    kernel_trace.setArg(3, d_voxelGrid);
    kernel_trace.setArg(4, gridSizeX);
    kernel_trace.setArg(5, gridSizeY);
    kernel_trace.setArg(6, gridSizeZ);
    kernel_trace.setArg(7, (float)gridOrigin.x);
    kernel_trace.setArg(8, (float)gridOrigin.y);
    kernel_trace.setArg(9, (float)gridOrigin.z);
    kernel_trace.setArg(10, voxelScale);
    kernel_trace.setArg(11, d_textures);
    kernel_trace.setArg(12, d_matrices);
    // 相机参数
    kernel_trace.setArg(13, (float)position.x);
    kernel_trace.setArg(14, (float)position.y);
    kernel_trace.setArg(15, (float)position.z);
    kernel_trace.setArg(16, (float)eye_dir.x);
    kernel_trace.setArg(17, (float)eye_dir.y);
    kernel_trace.setArg(18, (float)eye_dir.z);
    kernel_trace.setArg(19, (float)screen_x_dir.x);
    kernel_trace.setArg(20, (float)screen_x_dir.y);
    kernel_trace.setArg(21, (float)screen_x_dir.z);
    kernel_trace.setArg(22, (float)screen_y_dir.x);
    kernel_trace.setArg(23, (float)screen_y_dir.y);
    kernel_trace.setArg(24, (float)screen_y_dir.z);
    kernel_trace.setArg(25, (float)screen_x_wid);  // 需要传入全局常量
    kernel_trace.setArg(26, (float)screen_y_wid);
    kernel_trace.setArg(27, (int)render_distance * 2); // maxSteps
    kernel_trace.setArg(28, (int)mirror_times);        // maxBounce

    // 执行内核
    cl::NDRange global(width, height);
    queue.enqueueNDRangeKernel(kernel_trace, cl::NullRange, global, cl::NullRange);
    queue.finish();

    // 读回结果
    std::vector<unsigned int> gpuOutput(totalPixels);
    queue.enqueueReadBuffer(d_output, CL_TRUE, 0, sizeof(unsigned int) * totalPixels, gpuOutput.data());

    // 填充 current_frame（注意行翻转：OpenCL 图像原点在左上，原程序可能原点在左下）
    // 原程序 j 从 resolution_y 向下遍历，current_frame 索引为 (height-1-j)*width + i
    // 为了保持一致，我们直接按 OpenCL 的原样写入，然后在 optimized_render 中再调整。
    // 这里采用兼容原程序的索引：j 越大 y 坐标越小（屏幕下方）
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            // OpenCL 输出 y=0 对应原程序 j=resolution_y（屏幕最上方）
            int idx = j * width + i;
            unsigned int col = gpuOutput[idx];
            int r = (col >> 16) & 0xFF;
            int g = (col >> 8) & 0xFF;
            int b = col & 0xFF;
            // 原程序 current_frame 的存储顺序是 (resolution_y - j) 行最先
            int targetIdx = (height - 1 - j) * width + i;
            current_frame[targetIdx] = point(r, g, b);
        }
    }
}

// 全局 GPU 加速器实例
GPUAccelerator g_gpu;
#endif






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////工具函数
 
node round_self(node n){return {floor(tens*n.x+0.5)/tens,floor(tens*n.y+0.5)/tens,floor(tens*n.z+0.5)/tens};}
double sinn(double x){return floor(tens*sin(x/360*(2*M_PI))+0.5)/tens;}
double coss(double x){return floor(tens*cos(x/360*(2*M_PI))+0.5)/tens;}
point to_point(node x){return {floor(x.x),floor(x.y),floor(x.z)};}
node to_node(point x){return {x.x,x.y,x.z};}
point color_ll_to_point(ll color){
	if(color>=0)return {color>>16,(color>>8)&((1<<8)-1),color&((1<<8)-1)};
	else return {color,color,color};
}
ll color_point_to_ll(point color){
	if(color.x<0 && color.y<0 && color.z<0)return (color.x+color.y+color.z)/3;
	return ((color.x)<<16)+((color.y)<<8)+((color.z));
}
ll choose_which_face(node n,node direction){
	if(dot(n,direction)<0)n=zzz-n;
	double dt[3]={dot(n,{1,0,0}),dot(n,{0,1,0}),dot(n,{0,0,1})};
	ll inlight=0;
	for(ll i=1;i<3;i++)if(abs(dt[i])>abs(dt[inlight]))inlight=i;
	return inlight*2+(dt[inlight]<0)+1;
}
ll select_axis(node t) {//选择前进的轴 
	double min_t=INF;
	ll axis=0;
	if(t.x>=0&&t.x<min_t){min_t=t.x;axis=1;}
	if(t.y>=0&&t.y<min_t){min_t=t.y;axis=2;}
	if(t.z>=0&&t.z<min_t){min_t=t.z;axis=3;}
	return axis;
}
string downsample(const string&input,ll target_rows,ll target_cols) {
	if(target_rows<=0||target_cols<=0) {return "";}
	vector<string>lines;
	size_t start=0;
	size_t end;
	while((end=input.find('\n',start))!=string::npos){
		lines.push_back(input.substr(start,end-start));
		start=end+1;
	}
	if(start<input.size())lines.push_back(input.substr(start));
	else if(start==input.size()&&!input.empty()&&input.back()=='\n')lines.push_back("");
	if(lines.empty())return "";

	ll H=static_cast<ll>(lines.size());
	ll W=static_cast<ll>(lines[0].size());
	for(const auto&line:lines)if(static_cast<ll>(line.size())>W)W=static_cast<ll>(line.size());
	for(auto&line:lines)if(static_cast<ll>(line.size())<W)line.append(W-line.size(),' ');
	ll step_row=(H+target_rows-1)/target_rows;
	ll step_col=(W+target_cols-1)/target_cols;
	if (step_row<1)step_row=1;
	if (step_col<1)step_col=1;
	vector<string>result_lines;
	for(ll i=0;i<H;i+=step_row){
		string new_line;
		for(ll j=0;j<W;j+=step_col)new_line.push_back(lines[i][j]);
		result_lines.push_back(new_line);
	}

	string result;
	for(size_t k=0;k<result_lines.size();++k) {
		if(k!=0)result+='\n';
		result+=result_lines[k];
	}
	return result;
}
node rotate(node nd, ll op, bool is_rev) {


// ==================== 核心旋转/反射函数 ====================
// 功能：对方块内部的局部坐标 nd 进行正交变换，变换中心为 (0.5, 0.5, 0.5)
// 参数：
//   nd   : 方块内部的局部坐标，期望范围 [0,1]×[0,1]×[0,1]
//   op   : 0~47 的整数，按 6 个面 × 8 个稳定子分组
//   is_rev: true 表示应用逆变换（用于纹理采样时解旋）
// 返回：经过变换后的局部坐标（理想情况下仍在 [0,1] 内，反射可能超出但幅度很小）

	// 1. 平移到原点
	node nd1=nd-NODE_CUBE_CENTER;

	// 2. 选择变换矩阵（正向或逆向）
	const double (*mat)[3] = is_rev ? matrix_T_inv[op] : matrix_T[op];

	// 3. 应用线性变换
	node nd2;
	nd2.x = mat[0][0] * nd1.x + mat[0][1] * nd1.y + mat[0][2] * nd1.z;
	nd2.y = mat[1][0] * nd1.x + mat[1][1] * nd1.y + mat[1][2] * nd1.z;
	nd2.z = mat[2][0] * nd1.x + mat[2][1] * nd1.y + mat[2][2] * nd1.z;

	// 4. 平移回原位
	return nd2+NODE_CUBE_CENTER;
}
special_cube cube_rot(special_cube cube_num,ll op){
	for(ll i=0;i<cube_num.cube_face_num;i++)for(ll j=0;j<4;j++)cube_num._cube_face[i].X[j]=rotate(cube_num._cube_face[i].X[j],op,0);
	for(ll i=0;i<cube_num.mirror_round_num;i++)cube_num._mirror_round[i].X=rotate(cube_num._mirror_round[i].X,op,0);
	for(ll i=0;i<cube_num.part_round_num;i++){
		cube_num._part_round[i].X=rotate(cube_num._part_round[i].X,op,0);
		for(ll j=0;j<4;j++)cube_num._part_round[i].L[j]=rotate(cube_num._part_round[i].L[j]+NODE_CUBE_CENTER,op,0)-NODE_CUBE_CENTER;
	} 
	return cube_num;
}
// ----- 3D Perlin 噪声实现（改进版） -----
class PerlinNoise {
public:
    PerlinNoise() {
        // 初始化排列表
        for (int i = 0; i < 256; ++i) p[i] = i;
        std::random_shuffle(p, p + 256);
        for (int i = 0; i < 256; ++i) p[256 + i] = p[i];
    }

    // 获取 (x,y,z) 处的噪声值，返回值通常在 [-1,1] 之间
    double noise(double x, double y, double z) const {
        int X = (int)floor(x) & 255;
        int Y = (int)floor(y) & 255;
        int Z = (int)floor(z) & 255;

        x -= floor(x);
        y -= floor(y);
        z -= floor(z);

        double u = fade(x);
        double v = fade(y);
        double w = fade(z);

        int A  = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
        int B  = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;

        return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z)),
                               lerp(u, grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z))),
                       lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1)),
                               lerp(u, grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))));
    }

    // 分形布朗运动（多层噪声叠加）
    double fbm(double x, double y, double z, int octaves = 4, double lacunarity = 2.0, double gain = 0.5) const {
        double total = 0.0, amplitude = 1.0, frequency = 1.0, maxVal = 0.0;
        for (int i = 0; i < octaves; ++i) {
            total += noise(x * frequency, y * frequency, z * frequency) * amplitude;
            maxVal += amplitude;
            amplitude *= gain;
            frequency *= lacunarity;
        }
        return total / maxVal; // 归一化到大致[-1,1]
    }

private:
    int p[512];
    static double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    static double lerp(double t, double a, double b) { return a + t * (b - a); }
    static double grad(int hash, double x, double y, double z) {
        int h = hash & 15;
        double u = h < 8 ? x : y;
        double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }
};

// 全局噪声实例（可在 over_all_init 中初始化）
PerlinNoise perlin3D;
// 获取任意坐标的密度值（假设 Z 轴向上为正，高 Z ＝ 高海拔）
double get_density(point pos) {
    // 平滑的 3D 噪声，频率低、八度数适中，产生平缓丘陵
    double density = perlin3D.fbm(pos.x * 0.015, pos.y * 0.015, pos.z * 0.015,
                                  4,        // octaves
                                  2.0,      // lacunarity
                                  0.5);     // gain
    // 基础高度偏移：使地表大致位于 z=10 附近，0.05 的系数让坡度更缓
    density += (10.0 - pos.z) * 0.05;
    return density;
}
//ll generate_block(point pos){// original plan
//	if(pos.z>-14)return 0;
//	else if(pos.z==-14)return rev_idcube["grass"];
//	else return rev_idcube["stone"];
//}
ll generate_block(point pos) {//plan A 
    double dens = get_density(pos);
    const double THRESHOLD = 0.5;   // 密度阈值，大于此值生成固体

    if (dens > THRESHOLD) {
        // 检查上方三格是否至少有一格为空气（密度 ≤ 阈值）
        bool surface_nearby =
            (get_density({pos.x, pos.y, pos.z + 1}) <= THRESHOLD) ||
            (get_density({pos.x, pos.y, pos.z + 2}) <= THRESHOLD) ||
            (get_density({pos.x, pos.y, pos.z + 3}) <= THRESHOLD);

        if (surface_nearby)
            return rev_idcube["grass"];   // 表面三格用草方块
        else
            return rev_idcube["stone"];   // 内部用石头
    }
    return 0;   // 空气
}
//ll generate_block(point pos) {//plan B
//    // 使用三维噪声生成密度场
//    double density = perlin3D.fbm(pos.x * 0.03, pos.y * 0.03, pos.z * 0.03, 4, 2.0, 0.5);
//    // 调整密度，使底部更实，顶部更空
//    density += (15.0 - pos.z) * 0.1;  // 假设 z 向上为正？根据你的坐标系统调整方向
//
//    if (density > 0.2) {            // 密度阈值，可调
//        // 判断是否为表面：检查周围是否有空气方块
//        bool is_surface = (perlin3D.fbm((pos.x+1)*0.03, pos.y*0.03, pos.z*0.03, 4) < 0.2 ||
//                           perlin3D.fbm((pos.x-1)*0.03, pos.y*0.03, pos.z*0.03, 4) < 0.2 ||
//                           perlin3D.fbm(pos.x*0.03, (pos.y+1)*0.03, pos.z*0.03, 4) < 0.2 ||
//                           perlin3D.fbm(pos.x*0.03, (pos.y-1)*0.03, pos.z*0.03, 4) < 0.2 ||
//                           perlin3D.fbm(pos.x*0.03, pos.y*0.03, (pos.z+1)*0.03, 4) < 0.2 ||
//                           perlin3D.fbm(pos.x*0.03, pos.y*0.03, (pos.z-1)*0.03, 4) < 0.2);
//        if (is_surface) return rev_idcube["grass"];   // 表面草方块
//        else            return rev_idcube["stone"];   // 内部石头
//    }
//    return 0; // 空气
//}
ll generate_voxel(ll lod,point pos){
	if(lod==0){
		ll block_kind=generate_block(pos);
		cube[lod][pos]=block_kind;
		return (block_kind>0);
	}
	else{
		ll sum=0;
		for(ll i=0;i<VERTEX_NUM;i++){
			sum+=generate_voxel(lod-1,(pos*2ll)+(point){(i>>2)&1,(i>>1)&1,(i>>0)&1});
		}
		cube[lod][pos]=sum;
		return sum;
	}
}
ll get_block(ll lod,const point&pos){
	auto it=cube[lod].find(pos);
	if(it!=cube[lod].end())return it->second;
	point largest_voxel=to_point(pos*1.0/pow_2[MAX_LOD-lod]);
	generate_voxel(MAX_LOD,largest_voxel);
	return cube[lod][pos];
}
ll get_rotate(const point&pos){
	auto it=rot_state.find(pos);
	return (it!=rot_state.end())?it->second:0;
}
void save_frame_as_bmp(const string& filename,int32_t width, int32_t height,const vector<point>& frame_data){
	if (frame_data.size() < (size_t)(width * height)) return;

	// 新高度为原来的 2 倍
	int32_t new_height = height * 2;

	// 每行字节数（必须为4的倍数）
	int32_t bytes_per_line = (width * 3 + 3) & ~3;
	int32_t image_size = bytes_per_line * new_height;

	BMPFileHeader file_header;
	BMPInfoHeader info_header;

	info_header.biWidth = width;
	info_header.biHeight = new_height;	   // 拉伸后的高度
	info_header.biSizeImage = image_size;

	file_header.bfSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + image_size;

	ofstream ofs(filename, ios::binary);
	if (!ofs) return;

	ofs.write(reinterpret_cast<const char*>(&file_header), sizeof(BMPFileHeader));
	ofs.write(reinterpret_cast<const char*>(&info_header), sizeof(BMPInfoHeader));

	// 准备一行缓冲（BGR 顺序）
	vector<uint8_t> line_buffer(bytes_per_line, 0);

	// BMP 要求第一行是图像的最后一行，所以从原图底部开始扫描
	for (int32_t y = height - 1; y >= 0; --y) {
		// 填充当前原行的像素到 line_buffer
		for (int32_t x = 0; x < width; ++x) {
			size_t idx = y * width + x;
			point color = frame_data[idx];
			uint8_t r = color.x;
			uint8_t g = color.y;
			uint8_t b = color.z;

			line_buffer[x * 3 + 0] = b;
			line_buffer[x * 3 + 1] = g;
			line_buffer[x * 3 + 2] = r;
		}

		// 竖直拉伸：将同一行写入两次
		ofs.write(reinterpret_cast<const char*>(line_buffer.data()), bytes_per_line);
		ofs.write(reinterpret_cast<const char*>(line_buffer.data()), bytes_per_line);
	}

	ofs.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////系统函数

void HideCursor(){//隐藏光标 
	HANDLE consoleHandle=GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO information;
	information.dwSize=100;
	information.bVisible=FALSE;
	SetConsoleCursorInfo(consoleHandle,&information);
}
void CaptureMouseMovement(ll& dx,ll& dy){//捕捉鼠标运动 
	POINT pt;
	GetCursorPos(&pt);
	dx=pt.x-lastX;
	dy=pt.y-lastY;
}
void gotoxy(ll x,ll y){//光标移动 
	COORD pos={(short)x,(short)y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),pos);
	return;
}
void ScrollWindowToCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;

    SHORT cursorRow = csbi.dwCursorPosition.Y;
    SHORT windowHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    SHORT bufferHeight = csbi.dwSize.Y;

    SHORT newTop = cursorRow;
    if (newTop + windowHeight > bufferHeight)
        newTop = bufferHeight - windowHeight;
    if (newTop < 0) newTop = 0;

    SMALL_RECT newWindow = csbi.srWindow;
    newWindow.Top = newTop;
    newWindow.Bottom = newTop + windowHeight - 1;
    SetConsoleWindowInfo(hConsole, TRUE, &newWindow);
}
bool EnableVirtualTerminal(){//控制台右键、左键 
	HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);
	if(hOut==INVALID_HANDLE_VALUE)return false;
	DWORD dwMode=0;
	if(!GetConsoleMode(hOut,&dwMode))return false;
	dwMode|=0x0004;
	if(!SetConsoleMode(hOut,dwMode))return false;
	return true;
}
ll getTimeOfDayMicros(){//获得微秒时间 
	struct timeval tv;
	gettimeofday(&tv,NULL);
	ll microseconds=tv.tv_sec*1000000LL+tv.tv_usec;
	return microseconds;
}
ll get_nowtime(){//获得当前时间 
	time_t now=time(0);
	tm* ltm=localtime(&now);
	ll nowtime=(ll)(1900+ltm->tm_year)*10000000000ll+(ll)(1+ltm->tm_mon)*100000000ll+(ll)(ltm->tm_mday)*1000000ll+(ll)(ltm->tm_hour)*10000ll+(ll)(ltm->tm_min)*100+(ltm->tm_sec);
	return nowtime;
}
vector<string>getSaveFileNames(){//获得文件夹中文件名 
	vector<string> fileNames;
	const string folderPath="./saves";
	DIR*dir=opendir(folderPath.c_str());
	if(dir==NULL)return fileNames;
	dirent*entry;
	while((entry=readdir(dir))!=NULL){
		if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0)continue;
		string fullPath=folderPath+"/"+entry->d_name;
		struct stat fileStat;
		if(stat(fullPath.c_str(),&fileStat))continue;
		if(S_ISREG(fileStat.st_mode))fileNames.push_back(entry->d_name);
	}
	closedir(dir);
	sort(fileNames.begin(),fileNames.end());
	return fileNames;
}
void mouse(){//鼠标检测 
	ll dx=0,dy=0;
	CaptureMouseMovement(dx,dy);
	if(abs(dx)>DEADZONE||abs(dy)>DEADZONE){
		view_a-=dx*mouse_sensitivity;
		view_b-=dy*mouse_sensitivity;
		if(view_b>90.0)view_b=90.0;
		if(view_b<-90.0)view_b=-90.0;
		view_a=fmod(view_a,360.0);
		if(view_a<0)view_a+=360.0;
	}
	RECT consoleRect={0};
	if(g_hConsoleWnd)GetWindowRect(g_hConsoleWnd,&consoleRect);
	if(consoleRect.left==0&&consoleRect.right==0&&consoleRect.top==0&&consoleRect.bottom==0){
		ll screenWidth=GetSystemMetrics(SM_CXSCREEN);
		ll screenHeight=GetSystemMetrics(SM_CYSCREEN);
		consoleRect.left=0;
		consoleRect.top=0;
		consoleRect.right=screenWidth;
		consoleRect.bottom=screenHeight;
	}
	ll centerX=(consoleRect.left+consoleRect.right)/2;
	ll centerY=(consoleRect.top+consoleRect.bottom)/2;
	SetCursorPos(centerX,centerY);
	lastX=centerX;
	lastY=centerY;
	Sleep(1);
	leftDown=(GetAsyncKeyState(VK_LBUTTON)&0x8000)!=0;
	rightDown=(GetAsyncKeyState(VK_RBUTTON)&0x8000)!=0;
	if(leftDown&&!lastLeft&&is_mouse){
		delete_cube(eyegogogo(eye_dir));
		change=1;
	}
	if(rightDown&&!lastRight&&is_mouse){
		add_cube(eyegogogo(eye_dir),shortcut_key[now_cube]);
		change=1;
	}
	lastLeft=leftDown;
	lastRight=rightDown;
}
void keyboard(){//键盘检测 
	if(key_ch=='-'){
		if(resolution>1){//&&!(!is_color&&is_mouse)
			force_redraw=true;
			--resolution;
		}
	}
	else if(key_ch=='='){
		if(resolution<8){//&&!(!is_color&&is_mouse)
			force_redraw=true;
			++resolution;
		}
	}
	else if(key_ch=='['&&view_r_state>1)view_r=view_r_opt[--view_r_state];
	else if(key_ch==']'&&view_r_state<11)view_r=view_r_opt[++view_r_state];
	else if(key_ch=='w')position=position+(eye_dir*forbackward_step);
	else if(key_ch=='s')position=position-(eye_dir*forbackward_step);
	else if(key_ch=='a')position=position-(screen_x_dir*leftright_step);
	else if(key_ch=='d')position=position+(screen_x_dir*leftright_step);
	else if(key_ch=='p')position.z+=forbackward_step;
	else if(key_ch=='l')position.z-=forbackward_step;
	else if(key_ch=='t')position=position+(eye_xydir*forbackward_step);
	else if(key_ch=='g')position=position-(eye_xydir*forbackward_step);
	else if(key_ch=='r'&&view_b+updown_degree<=90)view_b+=updown_degree;
	else if(key_ch=='f'&&view_b-updown_degree>=-90)view_b-=updown_degree;
	else if(key_ch=='q'){
		view_a+=leftright_degree;
		if(view_a>360)view_a-=360;
	}
	else if(key_ch=='e'){
		view_a-=leftright_degree;
		if(view_a<0)view_a+=360;
	}
	else if(key_ch=='b'){
		add_cube(eyegogogo(eye_dir),shortcut_key[now_cube]);
		change=1;
	}
	else if(key_ch=='v'){
		delete_cube(eyegogogo(eye_dir));
		change=1;
	}
	else if(key_ch=='x'){
		add_cube(mirror_eyegogogo(position,eye_dir,mirror_times),shortcut_key[now_cube]);
		change=1;
	}
	else if(key_ch=='z'){
		delete_cube(mirror_eyegogogo(position,eye_dir,mirror_times));
		change=1;
	}
	else if(key_ch>='1'&&key_ch<='9')now_cube=key_ch-'0';
	else if(key_ch=='n'){
		pair<ll,point>pid=eyegogogo(eye_dir);
		point cube_num=pid.second;
		if(pid.first>0){
			rot_state[cube_num]=rot_state[cube_num]%VERTEX_NUM+(rot_state[cube_num]/VERTEX_NUM+1)%FACE_NUM*VERTEX_NUM;
			if(rot_state[cube_num]==0)rot_state.erase(cube_num);
			change=1;
		}
	}
	else if(key_ch=='m'){
		pair<ll,point>pid=eyegogogo(eye_dir);
		point cube_num=pid.second;
		if(pid.first>0){
			rot_state[cube_num]=(rot_state[cube_num]%VERTEX_NUM+1)%VERTEX_NUM+(rot_state[cube_num]/VERTEX_NUM)*VERTEX_NUM;
			if(rot_state[cube_num]==0)rot_state.erase(cube_num);
			change=1;
		}
	}
	else if(key_ch=='h'){
		is_color=!is_color;
		force_redraw=true;
	}
	else if(key_ch=='y')is_round=!is_round;
	else if(key_ch=='u'){
		is_mouse=!is_mouse;
		if(is_mouse){
			RECT rect={0};
			if(g_hConsoleWnd)GetWindowRect(g_hConsoleWnd,&rect);
			if(rect.left==0&&rect.right==0&&rect.top==0&&rect.bottom==0){
				ll screenWidth=GetSystemMetrics(SM_CXSCREEN);
				ll screenHeight=GetSystemMetrics(SM_CYSCREEN);
				rect.left=0;
				rect.top=0;
				rect.right=screenWidth;
				rect.bottom=screenHeight;
			}
			ll centerX=(rect.left+rect.right)/2;
			ll centerY=(rect.top+rect.bottom)/2;
			SetCursorPos(centerX,centerY);
			Sleep(15);
			SetCursorPos(centerX,centerY);
			Sleep(5);
			SetCursorPos(centerX,centerY);
			lastX=centerX;
			lastY=centerY;
			ShowCursor(FALSE);
			ClipCursor(&rect);
		}
		else {
			ShowCursor(TRUE);
			ClipCursor(NULL);
		}
		force_redraw=true;
	}
	else if(key_ch=='c')touch_c(get_nowtime());
	else if(key_ch=='k')touch_k(get_nowtime());
	else if(key_ch=='`'){
		map<point,ll>::iterator it;
		string adr="2_pow_world.txt";
		FILE *fp=fopen(adr.c_str(),"w");
		if(fp==NULL)return;
		fprintf(fp,"\noriginal:\n\n");
		for(it=cube[0].begin();it!=cube[0].end();it++)if((*it).second!=0)fprintf(fp,"%lld %lld %lld	%s\n",(ll)(*it).first.x,(ll)(*it).first.y,(ll)(*it).first.z,idcube[(*it).second].c_str());
		for(ll i=1;i<=MAX_LOD;i++){
			fprintf(fp,"\n%lld\n\n",i);
			for(it=cube[i].begin();it!=cube[i].end();it++){
				if((*it).second!=0)fprintf(fp,"%lld %lld %lld	%lld\n",(ll)(*it).first.x*pow_2[i],(ll)(*it).first.y*pow_2[i],(ll)(*it).first.z*pow_2[i],(*it).second);
			}
		}
		fclose(fp);
		info="2_pow_world is saved at "+to_string(get_nowtime());
		info_time=getTimeOfDayMicros();
	}
	else if(key_ch=='j'){
		vector<string>fileNames=getSaveFileNames();
		ll pos=0;
		bool pgstt=1;
		displaySaveList(fileNames,pos);
		while(1){
			if(_kbhit()){
				ll ch2=_getch();
				if(ch2=='-')pgstt=0;
				else if(ch2=='=')pgstt=1;
				else if(ch2=='w'||ch2=='s'){
					if(ch2=='w'&&pos>0)--pos;
					if(ch2=='s'&&pos<(ll)fileNames.size())++pos;
					displaySaveList(fileNames,pos);
				}
				else if(ch2>='0'&&ch2<='9'){
					ll dlt=pow_2[ch2-'0'];
					if(!pgstt&&pos-dlt>=0)pos-=dlt;
					if(pgstt&&pos+dlt<=(ll)fileNames.size())pos+=dlt;
					displaySaveList(fileNames,pos);
				}
				else if(ch2=='\r'){ // 回车
					if(change)touch_k(get_nowtime());
					if(pos==(ll)fileNames.size())break; // 选择 exit
					loadSave(fileNames[pos]);
					break;
				}
			}
		}
		force_redraw=true;
	}
	else if(key_ch=='\t'){
		ll pos=1;
		display_shortcut(pos);
		while(1){
			if(_kbhit()){
				ll ch2=_getch();
				if(ch2=='w'&&pos>1){
					pos--;
					display_shortcut(pos);
				}
				else if(ch2=='s'&&pos<kind_of_cube){
					pos++;
					display_shortcut(pos);
				}
				else if(ch2>='1'&&ch2<='9'&&cube_to_shortcut[pos]==0){
					cube_to_shortcut[shortcut_key[ch2-'0']]=0;
					shortcut_key[ch2-'0']=pos;
					cube_to_shortcut[pos]=ch2-'0';
					display_shortcut(pos);
				}
				else if(ch2>='1'&&ch2<='9'&&cube_to_shortcut[pos]!=ch2-'0'){
					swap(shortcut_key[ch2-'0'],shortcut_key[cube_to_shortcut[pos]]);
					swap(cube_to_shortcut[shortcut_key[ch2-'0']],cube_to_shortcut[shortcut_key[cube_to_shortcut[pos]]]);
					display_shortcut(pos);
				}
				else if(ch2=='\t')break;
			}
		}
		force_redraw=true;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////初始化函数 

void init_transform_matrices(){
//	R_face[face]:将参考面(z=1)映射到第 face 个面
//	其中face=0对应恒等映射(z=1面本身)
//	face 0:z=1 face 1:x=1 face 2:x=0 face 3:y=1 face 4:y=0 face 5:z=0
//	S[stab]:保持参考面(z=1)不变的8个变换(4个旋转+4个反射)
//	stab 0:恒等 stab 1:绕z轴90° stab 2:绕z轴180° stab 3:绕z轴270° stab 4:关于平面x=0反射 stab 5:关于平面y=0反射 stab 6:关于平面x=y反射 stab 7:关于平面x=-y反射
//	组合T[face*8+stab]=R_face[face]*S[stab]
//	矩阵乘法T=R_face*S
//	正交矩阵的逆=转置
	const double R_face[FACE_NUM][3][3]={
		{{1,0,0},{0,1,0},{0,0,1}},
		{{0,0,1},{0,1,0},{-1,0,0}},
		{{0,0,-1},{0,1,0},{1,0,0}},
		{{1,0,0},{0,0,1},{0,-1,0}},
		{{1,0,0},{0,0,-1},{0,1,0}},
		{{1,0,0},{0,-1,0},{0,0,-1}}
	};
	const double S[VERTEX_NUM][3][3]={
		{{1,0,0},{0,1,0},{0,0,1}},
		{{0,-1,0},{1,0,0},{0,0,1}},
		{{-1,0,0},{0,-1,0},{0,0,1}},
		{{0,1,0},{-1,0,0},{0,0,1}},
		{{-1,0,0},{0,1,0},{0,0,1}},
		{{1,0,0},{0,-1,0},{0,0,1}},
		{{0,1,0},{1,0,0},{0,0,1}},
		{{0,-1,0},{-1,0,0},{0,0,1}}
	};
	for(ll face=0;face<FACE_NUM;++face){
		for(ll stab=0;stab<VERTEX_NUM;++stab){
			ll idx=face*VERTEX_NUM+stab;
			for(ll i=0;i<3;++i){
				for(ll j=0;j<3;++j){
					matrix_T[idx][i][j]=0;
					for(ll k=0;k<3;++k){
						matrix_T[idx][i][j]+=R_face[face][i][k]*S[stab][k][j];
					}
				}
			}
			for(ll i=0;i<3;++i){
				for(ll j=0;j<3;++j){
					matrix_T_inv[idx][i][j]=matrix_T[idx][j][i];
				}
			}
		}
	}
}
void over_all_init(){//全体初始化 
	cout<<"init...";
	init_transform_matrices();
	FILE* fp=fopen("./texture/all_world_cube.txt","r");
	if(fp==NULL){
		info="fail to open all_world_cube.txt";
		info_time=get_nowtime();
	}
	fscanf(fp,"%lld",&kind_of_cube);
	
	sp_cube.resize(kind_of_cube+1);
	rot_sp_cube.resize(kind_of_cube+1,vector<special_cube>(FACE_NUM*VERTEX_NUM));
	idcube.resize(kind_of_cube+1);
	is_special.resize(kind_of_cube+1);
	all_cube_color.resize(kind_of_cube+1,vector<vector<vector<point> > >(FACE_NUM+2,vector<vector<point> >(TEX_SIZE,vector<point>(TEX_SIZE))));
	rev_idcube[""]=0;
	for(ll i=1;i<=kind_of_cube;i++){
		ll is_sp=0;
		char id[CHAR_MAXNUM];
		fscanf(fp,"%s %lld",id,&is_sp);
		string str_id(id);
		idcube[i]=str_id;
		is_special[i]=is_sp;
		rev_idcube[idcube[i]]=i;
	}
	fclose(fp);
	
	for(ll i=1;i<=kind_of_cube;++i){
		if(idcube[i].empty())continue;
		if(!is_special[i]){
			FILE* fp=fopen(("./texture/"+idcube[i]+".txt").c_str(),"r");
			if(fp==NULL)continue;
			for(ll j=1;j<=FACE_NUM;++j)for(ll ii=0;ii<TEX_SIZE;++ii)for(ll jj=0;jj<TEX_SIZE;++jj){
				ll tmp;
				fscanf(fp,"%lld",&tmp);
				all_cube_color[i][j][ii][jj]=color_ll_to_point(tmp);
			}
			fclose(fp);
		}
		else{
			FILE* fp=fopen(("./texture/"+idcube[i]+".txt").c_str(),"r");
			if(fp==NULL)continue;
			fscanf(fp,"%lld",&sp_cube[i].cube_face_num);
			sp_cube[i]._cube_face.resize(sp_cube[i].cube_face_num);
			for(ll j=0;j<sp_cube[i].cube_face_num;j++){
				for(ll k=0;k<4;k++)fscanf(fp,"%lf%lf%lf",&sp_cube[i]._cube_face[j].X[k].x,&sp_cube[i]._cube_face[j].X[k].y,&sp_cube[i]._cube_face[j].X[k].z);
				for(ll ii=0;ii<TEX_SIZE;ii++)for(ll jj=0;jj<TEX_SIZE;jj++){
					ll tmp;
					fscanf(fp,"%lld",&tmp);
					sp_cube[i]._cube_face[j].color[ii][jj]=color_ll_to_point(tmp);
				}
			}
			fscanf(fp,"%lld",&sp_cube[i].mirror_round_num);
			sp_cube[i]._mirror_round.resize(sp_cube[i].mirror_round_num);
			for(ll j=0;j<sp_cube[i].mirror_round_num;j++){
				fscanf(fp,"%lf%lf%lf",&sp_cube[i]._mirror_round[j].X.x,&sp_cube[i]._mirror_round[j].X.y,&sp_cube[i]._mirror_round[j].X.z);
				fscanf(fp,"%lf",&sp_cube[i]._mirror_round[j].r);
			}
			fscanf(fp,"%lld",&sp_cube[i].part_round_num);
			sp_cube[i]._part_round.resize(sp_cube[i].part_round_num);
			for(ll j=0;j<sp_cube[i].part_round_num;j++){
				fscanf(fp,"%lf%lf%lf",&sp_cube[i]._part_round[j].X.x,&sp_cube[i]._part_round[j].X.y,&sp_cube[i]._part_round[j].X.z);
				fscanf(fp,"%lf",&sp_cube[i]._part_round[j].r);
				for(ll k=0;k<4;k++)fscanf(fp,"%lf%lf%lf",&sp_cube[i]._part_round[j].L[k].x,&sp_cube[i]._part_round[j].L[k].y,&sp_cube[i]._part_round[j].L[k].z);
				for(ll ii=0;ii<TEX_SIZE;ii++)for(ll jj=0;jj<TEX_SIZE;jj++){
					ll tmp;
					fscanf(fp,"%lld",&tmp);
					sp_cube[i]._part_round[j].color[ii][jj]=color_ll_to_point(tmp);
				}
			}
			fclose(fp);
			
			
			for(ll j=0;j<FACE_NUM*VERTEX_NUM;j++)rot_sp_cube[i][j]=cube_rot(sp_cube[i],j);
		}
	}
	
	
	system("cls");
	cout << "init ok!";
	Sleep(20);
	EnableVirtualTerminal();
	prev_screen_color.resize((resolution_x[resolution]+1)*(resolution_y[resolution]+1),-1);
	current_frame.assign((resolution_x[resolution]+1)*(resolution_y[resolution]+1),0);
	string path_saves="./saves";
	string path_images="./images";
	string path_chunks="./chunks";
	filesystem::path folderPath=path_chunks.c_str();
	filesystem::remove_all(folderPath);
	CreateDirectory(path_saves.c_str(),NULL);
	CreateDirectory(path_images.c_str(),NULL);
	CreateDirectory(path_chunks.c_str(),NULL);
	
	for(ll i=-7;i<=7;++i)
		for(ll j=-7;j<=7;++j)
			for(ll k=-7;k<=7;++k)
				add_cube({CUBE_SELF,{i,j,k}},rev_idcube["grass"]);
	for(ll i=-6;i<=6;++i)
		for(ll j=-6;j<=6;++j)
			for(ll k=-6;k<=6;++k)
				add_cube({CUBE_SELF,{i,j,k}},rev_idcube[""]);
	for(ll i=-1;i<=1;i+=2)
		for(ll j=-2;j<=2;++j)
			for(ll k=-2;k<=2;++k)
				add_cube({CUBE_SELF,{i*6,j,k}},rev_idcube["stone"]);
	
	if(!g_hConsoleWnd)g_hConsoleWnd=FindWindowW(L"ConsoleWindowClass",NULL);
	RECT consoleRect={0};
	ll retry=0;
	while(retry<10){
		if(g_hConsoleWnd)GetWindowRect(g_hConsoleWnd,&consoleRect);
		if(consoleRect.left!=0||consoleRect.right!=0||consoleRect.top!=0||consoleRect.bottom!=0)break;
		Sleep(500);
		++retry;
	}
	ll initX,initY;
	if(consoleRect.left==0&&consoleRect.right==0&&consoleRect.top==0&&consoleRect.bottom==0){
		initX=GetSystemMetrics(SM_CXSCREEN)/2;
		initY=GetSystemMetrics(SM_CYSCREEN)/2;
	}
	else{
		initX=(consoleRect.left+consoleRect.right)/2;
		initY=(consoleRect.top+consoleRect.bottom)/2;
	}
	lastX=initX;
	lastY=initY;
}
void init(){//一帧初始化 
	image_s="";
	eye_xydir=(node){coss(view_a),sinn(view_a),0};
	eye_dir=(node){coss(view_b)*coss(view_a),coss(view_b)*sinn(view_a),sinn(view_b)};
	eye_act=eye_dir*view_r;
	screen_x_dir=(node){sinn(view_a),-coss(view_a),0};
	screen_y_dir=(node){-sinn(view_b)*coss(view_a),-sinn(view_b)*sinn(view_a),coss(view_b)};
	eye_xydir=round_self(eye_xydir);
	eye_dir=round_self(eye_dir);
	eye_act=round_self(eye_act);
	screen_x_dir=round_self(screen_x_dir);
	screen_y_dir=round_self(screen_y_dir);
	mr_pow[0]=1;
	for(ll i=1;i<=mirror_times;++i)mr_pow[i]=mirror_reduction*mr_pow[i-1];
	sky_now_time=getTimeOfDayMicros()%60000000/1000000.0;
	return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////找颜色函数 

point find_color(point cube_num,node cube_relative){//找方块对应位置颜色 
	ll cube_id=get_block(0,cube_num);
	point pixel_color;
	cube_relative=rotate(cube_relative,get_rotate(cube_num),1);
	double tmp[3]={cube_relative.x,cube_relative.y,cube_relative.z};
	ll inlight_direction=0;
	for(ll i=0;i<FACE_NUM;i++){
		if(abs(tmp[i/2]-(i%2))<EPSILON){
			inlight_direction=i+1;
			break;
		}
	}
	if(inlight_direction==0)return{0,0,0};
	pixel_color=all_cube_color[cube_id][inlight_direction][max(min((ll)floor(tmp[refer[(inlight_direction-1)/2][0]]*TEX_SIZE),TEX_SIZE-1),0ll)][max(min((ll)floor(tmp[refer[(inlight_direction-1)/2][1]]*TEX_SIZE),TEX_SIZE-1),0ll)];
	return pixel_color;
}
point hsl_rgb(double h,double s,double l){
	double c=(1-abs(2*l-1))*s;
	double x=c*(1-abs(fmod(h/60.0,2)-1));
	double m=l-c/2;
	double r,g,b;
	if(h<60){r=c;g=x;b=0;}
	else if(h<120){r=x;g=c;b=0;}
	else if(h<180){r=0;g=c;b=x;}
	else if(h<240){r=0;g=x;b=c;}
	else if(h<300){r=x;g=0;b=c;}
	else{r=c;g=0;b=x;}
	return{(r+m)*255,(g+m)*255,(b+m)*255};
}
point getSkyColor(node direction){
	double timeFactor=(sky_now_time)/60.0;
	double verangleFactor=atan2(direction.z,sqrt(direction.x*direction.x+direction.y*direction.y))/M_PI+0.5;
	double horangleFactor=atan2(direction.x,direction.y)/(M_PI);
	double h=timeFactor*360;
	double s=abs(horangleFactor);
	double l=verangleFactor;
	return hsl_rgb(h,s,l);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////一像素填充函数 
char find_char(node cube_relative){
	if(cube_relative.x==0)return dirchar[1];
	else if(cube_relative.x==1)return dirchar[2];
	else if(cube_relative.y==0)return dirchar[3];
	else if(cube_relative.y==1)return dirchar[4];
	else if(cube_relative.z==0)return dirchar[5];
	else if(cube_relative.z==1)return dirchar[6];
	else return '?';
}
ll find_face(node cube_relative){
	if(cube_relative.x==0)return 1;
	else if(cube_relative.x==1)return 2;
	else if(cube_relative.y==0)return 3;
	else if(cube_relative.y==1)return 4;
	else if(cube_relative.z==0)return 5;
	else if(cube_relative.z==1)return 6;
	else return 0;
}
void record_s(ll cube_id,ll inlight_direction){//添加字符
	if(idcube[cube_id]!="")image_s+=dirchar[inlight_direction];
	else if(inlight_direction)image_s+='_';
	else image_s+='|';
	return;
}
void record_s_by_char(char ch){
	image_s+=ch;
	return;
}
void print(point color,ll mr_times,ll pixel_idx){//添加像素颜色 
	current_frame[pixel_idx]=to_point(color*mr_pow[mirror_times-mr_times]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////体素计算函数 

node eye_next(node cur_position,node direction){
	node safe_pos=cur_position-direction*EPSILON;
	point c=to_point(safe_pos);
	for (ll lod=MAX_LOD;lod>=0;--lod) {
		ll scale=(lod==0)?1:pow_2[lod];
		point b=to_point(c*1.0/scale);
		bool empty;
		if(lod==0){
			empty=(get_block(0,c)==0);
		}
		else{
			empty=(get_block(lod,b)==0);
		}
		if(!empty&&lod>0)continue;
		node res=NODE_INVALID_D;
		double dirs[3]={direction.x,direction.y,direction.z};
		double pos[3]={cur_position.x,cur_position.y,cur_position.z};
		for(ll i=0;i<3;++i){
			if(fabs(dirs[i])<EPSILON)continue;
			double boundary;
			if(dirs[i]>0){
				if(lod==0)boundary=(i==0)?c.x+1.0:(i==1)?c.y+1.0:c.z+1.0;
				else boundary=(i==0)?(b.x+1)*scale:(i==1)?(b.y+1)*scale:(b.z+1)*scale;
			}
			else{
				if(lod==0)boundary=(i==0)?c.x:(i==1)?c.y:c.z;
				else boundary=(i==0)?b.x*scale:(i==1)?b.y*scale:b.z*scale;
			}
			double t=(boundary-pos[i])/dirs[i];
			if(t<EPSILON)t=EPSILON;
			if(i==0)res.x=t;
			else if(i==1)res.y=t;
			else res.z=t;
		}
		return res;
	}
	return NODE_INVALID_D;
}
void advance_ray(ll axis,node dir,node t_dist,node& pos,point& cube_num){//用指针完成前进1体素操作 
	double p[3]={pos.x,pos.y,pos.z};
	double d[3]={dir.x,dir.y,dir.z};
	double t[3]={t_dist.x,t_dist.y,t_dist.z};
	ll idx=axis-1;
	double step=t[idx];
	bool pos_dir=d[idx]>=0;
	for(ll i=0;i<3;++i)p[i]+=d[i]*step;
	ll c[3];
	for(ll i=0;i<3;++i){
		double offset=(i==idx)?(pos_dir?EPSILON:-EPSILON):0.0;
		c[i]=floor(p[i]+offset);
	}
	pos.x=p[0];pos.y=p[1];pos.z=p[2];
	cube_num.x=c[0];cube_num.y=c[1];cube_num.z=c[2];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////特殊方块判断函数 

bool rayTriangleIntersect(const node& orig, const node& dir,const node& v0, const node& v1, const node& v2,double& t, double& outU, double& outV){
	// 射线与三角形相交
	node edge1=v1-v0;
	node edge2=v2-v0;
	node pvec=cross(dir,edge2);
	double det=dot(edge1,pvec);
	if(fabs(det)<EPSILON)return false;
	double invDet=1.0/det;
	node tvec=orig-v0;
	outU=dot(tvec,pvec)*invDet;
	if(outU<0.0||outU>1.0)return false;
	node qvec=cross(tvec,edge1);
	outV=dot(dir,qvec)*invDet;
	if(outV<0.0||outU+outV>1.0)return false;
	t=dot(edge2,qvec)*invDet;
	return t>=-EPSILON;
}
pair<double, double> part_round_to_mn(const node& L1, const node& L2, const node& L3, const node& L4, const node& L5){
	// 确保 L5 是单位向量（输入应已经是单位向量，此处再次归一化）
	node dir = normalize(L5);

	// 1. 计算辅助向量
	node A = L2 - L3;
	node B = L4 - L3;
	node C = L1 - L2 - L4 + L3;

	// 2. 构造与 L5 正交的两个单位向量 u, v
	node u, v;
	if (fabs(dir.x) > EPSILON || fabs(dir.y) > EPSILON) {
		u = {-dir.y, dir.x, 0};
	} else {
		u = {0, -dir.z, dir.y};
	}
	u = normalize(u);
	v = cross(dir, u);
	v = normalize(v);

	// 3. 计算标量系数
	double a1 = dot(A, u), b1 = dot(B, u), c1 = dot(C, u), d1 = -dot(L3, u);
	double a2 = dot(A, v), b2 = dot(B, v), c2 = dot(C, v), d2 = -dot(L3, v);

	// 4. 构造关于 n 的二次方程: coeff_n2 * n^2 + coeff_n1 * n + coeff_n0 = 0
	double coeff_n2 = b2 * c1 - c2 * b1;
	double coeff_n1 = -b1 * a2 + b2 * a1 + c2 * d1 - d2 * c1;
	double coeff_n0 = d1 * a2 - d2 * a1;

	// 辅助函数：尝试用给定的 n 计算 m
	auto compute_m = [&](double n_val) -> double {
		double denom1 = a1 + n_val * c1;
		double denom2 = a2 + n_val * c2;
		if (fabs(denom1) > EPSILON) {
			return (d1 - n_val * b1) / denom1;
		} else if (fabs(denom2) > EPSILON) {
			return (d2 - n_val * b2) / denom2;
		} else {
			return INVALID_D;  // 退化情况
		}
	};

	// 解二次方程
	vector<double> n_candidates;
	if (fabs(coeff_n2) < EPSILON) {
		// 线性方程
		if (fabs(coeff_n1) > EPSILON) {
			double n = -coeff_n0 / coeff_n1;
			if (n >= -EPSILON && n <= 1.0 + EPSILON)
				n_candidates.push_back(n);
		}
	} else {
		double delta = coeff_n1 * coeff_n1 - 4.0 * coeff_n2 * coeff_n0;
		if (delta < -EPSILON) {
			// 无实数解，返回无效值
			return PDD_INVALID_D;
		}
		delta = max(delta, 0.0);
		double sqrt_delta = sqrt(delta);
		double n1 = (-coeff_n1 - sqrt_delta) / (2.0 * coeff_n2);
		double n2 = (-coeff_n1 + sqrt_delta) / (2.0 * coeff_n2);
		if (n1 >= -EPSILON && n1 <= 1.0 + EPSILON) n_candidates.push_back(n1);
		if (n2 >= -EPSILON && n2 <= 1.0 + EPSILON) n_candidates.push_back(n2);
	}

	// 对每个候选 n 计算 m，检查是否在 [0,1] 内
	for (double n_val : n_candidates) {
		n_val = max(0.0, min(1.0, n_val));
		double m_val = compute_m(n_val);
		if (m_val < -EPSILON || m_val > 1.0 + EPSILON) continue;
		m_val = max(0.0, min(1.0, m_val));

		// 重建曲面上的点（相对于 L3）
		node P = L3 + A * m_val + B * n_val + C * m_val * n_val;
		// 关键：方向一致性检查，只接受与射线方向同向的点
		if (dot(P,dir)<0)continue;

		return {m_val, n_val};
	}

	// 未找到有效解
	return PDD_INVALID_D;
}
void face_is_hit(cube_face cf,node p0,node v,cube_face&best_cf,double&dis,point&res_color,node&best_jd,ll&which_special){
	// 射线与四边形面片求交点，返回最近交点及纹理颜色
	// 四边形分解为两个三角形：X0,X1,X2和 X0,X2,X3
	node tri1[3]={cf.X[0],cf.X[1],cf.X[2]};
	node tri2[3]={cf.X[0],cf.X[2],cf.X[3]};
	double best_t=INF;
	double best_u=0.0,best_v=0.0;
	ll hit_tri=UNKNOWN;// 1:第一个三角形, 2:第二个三角形
	double t,u,v_local;
	if(rayTriangleIntersect(p0,v,tri1[0],tri1[1],tri1[2],t,u,v_local)){
		if(t<best_t){
			best_t=t;
			best_u=u;
			best_v=v_local;
			hit_tri=FIRST;
		}
	}
	if(rayTriangleIntersect(p0,v,tri2[0],tri2[1],tri2[2],t,u,v_local)){
		if(t<best_t){
			best_t=t;
			best_u=u;
			best_v=v_local;
			hit_tri=SECOND;
		}
	}
	if(hit_tri==UNKNOWN)return;//未相交
	if(best_t>=dis)return;//不是最近的交点
	double tex_s,tex_t;
	if(hit_tri==FIRST){// 三角形1 u X0->X1,v X0->X2
		tex_s=best_u+best_v;
		tex_t=best_v;
	}
	else{// 三角形2 u X0->X2,v X0->X3
		tex_s=best_u;
		tex_t=best_u+best_v;
	}
	if(cf.color[max(0ll,min(TEX_SIZE-1,(ll)(tex_s*TEX_SIZE)))][max(0ll,min(TEX_SIZE-1,(ll)(tex_t*TEX_SIZE)))]==INV)return;
	node jd=p0+v*best_t;
	if(jd.x<-EPSILON||jd.y<-EPSILON||jd.z<-EPSILON||jd.x-1>EPSILON||jd.y-1>EPSILON||jd.z-1>EPSILON)return;//外部的点不可见
	which_special=CUBE_fACE;
	best_jd=jd;
	best_cf=cf;
	dis=best_t;
	res_color=cf.color[max(0ll,min(TEX_SIZE-1,(ll)(tex_s*TEX_SIZE)))][max(0ll,min(TEX_SIZE-1,(ll)(tex_t*TEX_SIZE)))];
}
void ball_is_hit(mirror_round mr,node p0,node v,mirror_round&best_mr,double&dis,point&res_color,node&best_jd,ll&which_special){
	v=normalize(v);
	node to_ball=p0-mr.X;
	double fun_a=dot(v,v);
	double fun_b=2*dot(v,to_ball);
	double fun_c=dot(to_ball,to_ball)-mr.r*mr.r;
	if(fun_b*fun_b-4*fun_a*fun_c<0)return;//无交点 
	double t1=(-fun_b-sqrt(fun_b*fun_b-4*fun_a*fun_c))/(2*fun_a);//较小交点 
	double t2=(-fun_b+sqrt(fun_b*fun_b-4*fun_a*fun_c))/(2*fun_a);//较大交点 
	if(t2<EPSILON)return;
	double t=(t1>EPSILON?t1:t2);//进入球后，光线在里面出不去，全黑
	if(t>=dis)return;//如果大于距离，则不考虑此球面
	node tmpjd=p0+v/sqrt(fun_a)*t;
	if(tmpjd.x<-EPSILON||tmpjd.y<-EPSILON||tmpjd.z<-EPSILON||tmpjd.x-1>EPSILON||tmpjd.y-1>EPSILON||tmpjd.z-1>EPSILON)return;//落在外面不考虑,保证半球的渲染完整。 
	//更新 
	which_special=MIRROR_ROUND;
	best_mr=mr; 
	dis=t;
	res_color=REF;
	best_jd=p0+v/sqrt(fun_a)*t;
	return;
}
void part_is_hit(part_round pr,node p0,node v,part_round&best_pr,double&dis,point&res_color,node&best_jd,ll&which_special){
	v=normalize(v);
	node to_ball=p0-pr.X;
	double fun_a=dot(v,v);
	double fun_b=2*dot(v,to_ball);
	double fun_c=dot(to_ball,to_ball)-pr.r*pr.r;
	if(fun_b*fun_b-4*fun_a*fun_c<0)return;//无交点
	double t1=(-fun_b-sqrt(fun_b*fun_b-4*fun_a*fun_c))/(2*fun_a);//较小交点
	double t2=(-fun_b+sqrt(fun_b*fun_b-4*fun_a*fun_c))/(2*fun_a);//较大交点
	if(t2<EPSILON)return;
	point color_t1=INV;
	if(t1>EPSILON)
	{
		node tmpjd_t1=p0+v/sqrt(fun_a)*t1;
		pair<double,double>mn_t1=part_round_to_mn(pr.L[0],pr.L[1],pr.L[2],pr.L[3],tmpjd_t1-pr.X);
		if(mn_t1!=PDD_INVALID_D)
		{
			if(tmpjd_t1.x>=-EPSILON&&tmpjd_t1.y>=-EPSILON&&tmpjd_t1.z>=-EPSILON&&tmpjd_t1.x-1<=EPSILON&&tmpjd_t1.y-1<=EPSILON&&tmpjd_t1.z-1<=EPSILON)
			{
				color_t1=pr.color[max(min((ll)floor(mn_t1.first*TEX_SIZE),TEX_SIZE-1),0ll)][max(min((ll)floor(mn_t1.second*TEX_SIZE),TEX_SIZE-1),0ll)];
			}
		}
	}
	point color_t2=INV;
	if(t2>EPSILON)
	{
		node tmpjd_t2=p0+v/sqrt(fun_a)*t2;
		pair<double,double>mn_t2=part_round_to_mn(pr.L[0],pr.L[1],pr.L[2],pr.L[3],tmpjd_t2-pr.X);
		if(mn_t2!=PDD_INVALID_D)
		{
			if(tmpjd_t2.x>=-EPSILON&&tmpjd_t2.y>=-EPSILON&&tmpjd_t2.z>=-EPSILON&&tmpjd_t2.x-1<=EPSILON&&tmpjd_t2.y-1<=EPSILON&&tmpjd_t2.z-1<=EPSILON){
				color_t2=pr.color[max(min((ll)floor(mn_t2.first*TEX_SIZE),TEX_SIZE-1),0ll)][max(min((ll)floor(mn_t2.second*TEX_SIZE),TEX_SIZE-1),0ll)];
			}
		}
	}
	
	if(color_t1!=INV){
		if(t1<dis){
			which_special=PART_ROUND;
			best_pr=pr;
			dis=t1;
			best_jd=p0+v/sqrt(fun_a)*t1;
			if(color_t1==REF)res_color=REF;
			else if(color_t1<0)res_color=0;
			else res_color=color_t1;
		}
	}
	else if(color_t2!=INV){
		if(t2<dis){
			which_special=PART_ROUND;
			best_pr=pr;
			dis=t2;
			best_jd=p0+v/sqrt(fun_a)*t2;
			if(color_t2==REF)res_color=REF;
			else if(color_t2<0)res_color=0;
			else res_color=color_t2;
		}
	}
}
bool special_is_hit(point cube_num,node cube_relative,node direction,bool& is_mirror,node& n,node& jd,point&res_color){ 
	ll cube_type=get_block(0,cube_num);
	double dis=INF;
	ll which_special=0;
	res_color=INV;
	cube_face cf;
	mirror_round mr;
	part_round pr;
	for(ll i=0;i<rot_sp_cube[cube_type][get_rotate(cube_num)].cube_face_num;i++)face_is_hit(rot_sp_cube[cube_type][get_rotate(cube_num)]._cube_face[i],cube_relative,direction,cf,dis,res_color,jd,which_special);
	for(ll i=0;i<rot_sp_cube[cube_type][get_rotate(cube_num)].mirror_round_num;i++)ball_is_hit(rot_sp_cube[cube_type][get_rotate(cube_num)]._mirror_round[i],cube_relative,direction,mr,dis,res_color,jd,which_special);
	for(ll i=0;i<rot_sp_cube[cube_type][get_rotate(cube_num)].part_round_num;i++)part_is_hit(rot_sp_cube[cube_type][get_rotate(cube_num)]._part_round[i],cube_relative,direction,pr,dis,res_color,jd,which_special);
	if(which_special==CUBE_fACE){
		n=cross(cf.X[0]-cf.X[1],cf.X[0]-cf.X[2]);
		if(dot(n,direction)>0)n=zzz-n;
		n=normalize(n);
	}
	else if(which_special==MIRROR_ROUND){
		n=jd-mr.X;
		if(dot(n,direction)>0)n=zzz-n;
		n=normalize(n);
	}
	else if(which_special==PART_ROUND){
		n=jd-pr.X;
		if(dot(n,direction)>0)n=zzz-n;
		n=normalize(n);
	}
	
	if(res_color==INV)return false;
	else if(res_color==REF){
		is_mirror=1;
		return true;
	}
	
	
//	if(is_gogogo){
//		record_s(cube_type,choose_which_face(n,direction));
//		print(res_color,mr_times,pixel_idx);
//	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////3个gogogo函数 



//struct hit_cube{
//  point cube_num;
//	node v={1,0,0};
//	node p0={0,0,0};
//
//	
////返回： 
//	bool is_hit=0;
//	
//	
//	
//	bool is_mirror=0;
//	
//	
//	ll res_color=0;
//	char ch='?';
//	ll inlight_direction=0;
//	
//	node jd={0,0,0};
//	
//	node n={1,0,0};
//	node v_next={1,0,0};
//  
//  node p={0,0,0};
//};

//组合：
//① 
//is_hit=0
//

//② 
//is_hit=1
//is_mirror=0
//res_color=?
//ch=?
//inlight_direction=?
//

//③
//is_hit=1
//is_mirror=1
//inlight_direction=?
//jd=?
//n=?
//v_next=?
//p=? 


/*

hc.inlight_direction=choose_which_face(hc.n,hc.v)
*/
void calc_ray(hit_cube&hc){
	hc.p0=round_self(hc.p0);
	if(is_special[get_block(0,hc.cube_num)]){//over
		special_is_hit(hc.cube_num,hc.p0,hc.v,hc.is_mirror,hc.n,hc.jd,hc.res_color);
		
		hc.is_hit=(hc.res_color!=INV);
		//hc.is_mirror
		
		if(hc.is_hit){
			//hc.res_color
			hc.ch=dirchar[choose_which_face(hc.n,hc.v)];
			hc.inlight_direction=choose_which_face(hc.n,hc.v);
			
			//hc.jd
			
			//hc.n
			hc.v_next=hc.v-hc.n*2ll*dot(hc.v,hc.n);
			
			hc.p=hc.jd+hc.n*EPSILON;
		}
	}
	else if(get_block(0,hc.cube_num)==rev_idcube["mirror"]){//over
		hc.is_hit=1;
		if(!(hc.p0.x==0||hc.p0.x==1||hc.p0.y==0||hc.p0.y==1||hc.p0.z==0||hc.p0.z==1)){
			point end_face={(hc.v.x>0),(hc.v.y>0),(hc.v.z>0)};
			double dis[3]={INVALID_D,INVALID_D,INVALID_D};
			if(hc.v.x!=0)dis[0]=(end_face.x-hc.p0.x)/hc.v.x;
			if(hc.v.y!=0)dis[1]=(end_face.y-hc.p0.y)/hc.v.y;
			if(hc.v.z!=0)dis[2]=(end_face.z-hc.p0.z)/hc.v.z;
			
			ll axis=select_axis({dis[0],dis[1],dis[2]});
			if(axis==0){//over
				hc.is_mirror=0;
				hc.res_color=0;
				hc.ch='?';
				hc.inlight_direction=0;
				return;
			}
			else{//over
				node end_point=round_self(hc.p0+hc.v*dis[axis-1]);
				hc.p0=end_point;
			}
		}
		
		ll axis=0;
		if(hc.p0.x==0||hc.p0.x==1)axis=1;
		else if(hc.p0.y==0||hc.p0.y==1)axis=2;
		else axis=3;
		if(axis==1&&((abs(hc.p0.y-CUBE_CENTER)<MIRROR_CENTER&&abs(hc.p0.z-CUBE_CENTER)<MIRROR_CENTER)||get_block(0,hc.cube_num+(point){0,(hc.p0.y<CUBE_CENTER)?-1:1,0})==rev_idcube["mirror"]&&abs(hc.p0.z-CUBE_CENTER)<MIRROR_CENTER||get_block(0,hc.cube_num+(point){0,0,(hc.p0.z<CUBE_CENTER)?-1:1})==rev_idcube["mirror"]&&abs(hc.p0.y-CUBE_CENTER)<MIRROR_CENTER||abs(hc.p0.y-CUBE_CENTER)>=MIRROR_CENTER&&abs(hc.p0.z-CUBE_CENTER)>=MIRROR_CENTER&&get_block(0,hc.cube_num+(point){0,(hc.p0.y<CUBE_CENTER)?-1:1,0})==rev_idcube["mirror"]&&get_block(0,hc.cube_num+(point){0,0,(hc.p0.z<CUBE_CENTER)?-1:1})==rev_idcube["mirror"]&&get_block(0,hc.cube_num+(point){0,(hc.p0.y<CUBE_CENTER)?-1:1,(hc.p0.z<CUBE_CENTER)?-1:1})==rev_idcube["mirror"]))
		{//over
			hc.is_mirror=1;
			
			hc.jd=hc.p0;
			hc.n=(hc.v.x>0)?(node){-1,0,0}:(node){1,0,0};
			
			hc.inlight_direction=choose_which_face(hc.n,hc.v);//在n定义后 
			
			hc.v_next={-hc.v.x,hc.v.y,hc.v.z};
			hc.p=hc.p0+hc.v_next*EPSILON*2ll;
		}
		else if(axis==2&&((abs(hc.p0.x-CUBE_CENTER)<MIRROR_CENTER&&abs(hc.p0.z-CUBE_CENTER)<MIRROR_CENTER)||get_block(0,hc.cube_num+(point){(hc.p0.x<CUBE_CENTER)?-1:1,0,0})==rev_idcube["mirror"]&&abs(hc.p0.z-CUBE_CENTER)<MIRROR_CENTER||get_block(0,hc.cube_num+(point){0,0,(hc.p0.z<CUBE_CENTER)?-1:1})==rev_idcube["mirror"]&&abs(hc.p0.x-CUBE_CENTER)<MIRROR_CENTER||abs(hc.p0.x-CUBE_CENTER)>=MIRROR_CENTER&&abs(hc.p0.z-CUBE_CENTER)>=MIRROR_CENTER&&get_block(0,hc.cube_num+(point){(hc.p0.x<CUBE_CENTER)?-1:1,0,0})==rev_idcube["mirror"]&&get_block(0,hc.cube_num+(point){0,0,(hc.p0.z<CUBE_CENTER)?-1:1})==rev_idcube["mirror"]&&get_block(0,hc.cube_num+(point){(hc.p0.x<CUBE_CENTER)?-1:1,0,(hc.p0.z<CUBE_CENTER)?-1:1})==rev_idcube["mirror"]))
		{//over
			hc.is_mirror=1;
			
			hc.jd=hc.p0;
			hc.n=(hc.v.y>0)?(node){0,-1,0}:(node){0,1,0};
			
			hc.inlight_direction=choose_which_face(hc.n,hc.v);//在n定义后 
			
			hc.v_next={hc.v.x,-hc.v.y,hc.v.z};
			hc.p=hc.p0+hc.v_next*EPSILON*2ll;
		}
		else if(axis==3&&((abs(hc.p0.x-CUBE_CENTER)<MIRROR_CENTER&&abs(hc.p0.y-CUBE_CENTER)<MIRROR_CENTER)||get_block(0,hc.cube_num+(point){(hc.p0.x<CUBE_CENTER)?-1:1,0,0})==rev_idcube["mirror"]&&abs(hc.p0.y-CUBE_CENTER)<MIRROR_CENTER||get_block(0,hc.cube_num+(point){0,(hc.p0.y<CUBE_CENTER)?-1:1,0})==rev_idcube["mirror"]&&abs(hc.p0.x-CUBE_CENTER)<MIRROR_CENTER||abs(hc.p0.x-CUBE_CENTER)>=MIRROR_CENTER&&abs(hc.p0.y-CUBE_CENTER)>=MIRROR_CENTER&&get_block(0,hc.cube_num+(point){(hc.p0.x<CUBE_CENTER)?-1:1,0,0})==rev_idcube["mirror"]&&get_block(0,hc.cube_num+(point){0,(hc.p0.y<CUBE_CENTER)?-1:1,0})==rev_idcube["mirror"]&&get_block(0,hc.cube_num+(point){(hc.p0.x<CUBE_CENTER)?-1:1,(hc.p0.y<CUBE_CENTER)?-1:1,0})==rev_idcube["mirror"]))
		{//over
			hc.is_mirror=1;
			
			hc.jd=hc.p0;
			hc.n=(hc.v.z>0)?(node){0,0,-1}:(node){0,0,1};
			
			hc.inlight_direction=choose_which_face(hc.n,hc.v);//在n定义后 
			
			hc.v_next={hc.v.x,hc.v.y,-hc.v.z};
			hc.p=hc.p0+hc.v_next*EPSILON*2ll;
		}
		else{//over
			hc.is_mirror=0;
			
			hc.res_color=find_color(hc.cube_num,hc.p0);
			hc.ch=find_char(hc.p0);
			hc.inlight_direction=find_face(hc.p0);
		}
	}
	else if(get_block(0,hc.cube_num)==0){//over
		hc.is_hit=0;
	}
	else{//over
		hc.is_hit=1;
		hc.is_mirror=0;
		
		if(hc.p0.x==0||hc.p0.x==1||hc.p0.y==0||hc.p0.y==1||hc.p0.z==0||hc.p0.z==1){//over
			hc.res_color=find_color(hc.cube_num,hc.p0);
			hc.ch=find_char(hc.p0);
			hc.inlight_direction=find_face(hc.p0);
		}
		else{//over
			point end_face={(hc.v.x>0),(hc.v.y>0),(hc.v.z>0)};
			double dis[3]={INVALID_D,INVALID_D,INVALID_D};
			if(hc.v.x!=0)dis[0]=(end_face.x-hc.p0.x)/hc.v.x;
			if(hc.v.y!=0)dis[1]=(end_face.y-hc.p0.y)/hc.v.y;
			if(hc.v.z!=0)dis[2]=(end_face.z-hc.p0.z)/hc.v.z;
			
			ll axis=select_axis({dis[0],dis[1],dis[2]});
			if(axis==0){//over
				hc.res_color=0;
				hc.ch='?';
				hc.inlight_direction=0;
			}
			else{//over
				node end_point=round_self(hc.p0+hc.v*dis[axis-1]);
				hc.res_color=find_color(hc.cube_num,end_point);
				hc.ch=find_char(end_point);
				hc.inlight_direction=find_face(end_point);
			}
		}
	}
}
void gogogo(node tmp_position,node direction,ll mr_times,ll pixel_idx){//显示的像素 
	if(mr_times==0){
		record_s(0,(ll)(direction.z>=0));
		print({0,0,0},0,pixel_idx);
		return;
	}
	node cur_position=tmp_position;
	point cube_num=to_point(cur_position);
	node cube_relative=cur_position-cube_num;
	ll axis;
	
	while(manhattan_dis(to_point(tmp_position),to_point(cur_position))<=render_distance){
		node t_dist=eye_next(cur_position,direction);
		axis=select_axis(t_dist);
		if(axis<=0||axis>FACE_NUM)break;
		
		cube_relative=cur_position-cube_num;
		
		hit_cube hc;
		hc.cube_num=cube_num;
		hc.p0=cube_relative;
		hc.v=direction;
		calc_ray(hc);
		if(hc.is_hit){
			if(hc.is_mirror==0){
				record_s_by_char(hc.ch);
				print(hc.res_color,mr_times,pixel_idx);
				return;
			}
			else{
				gogogo(hc.cube_num+hc.p,hc.v_next,mr_times-1,pixel_idx);
				return;
			}
		}
		
		advance_ray(axis,direction,t_dist,cur_position,cube_num);
	}
	record_s(0,(ll)(direction.z>=0));
	print(getSkyColor(direction),mr_times,pixel_idx);
	return;
}
pair<ll,point>eyegogogo(node direction){//手看到的方块 
	if(get_block(0,to_point(position))&&!is_special[get_block(0,to_point(position))])return{CUBE_SELF,to_point(position)};//对于完整方块起作用，能在方块内判断是否击中自己方块 
	node cur_position=position;
	point cube_num=to_point(position);
	node cube_relative;
	
	while(manhattan_dis(to_point(cur_position),to_point(position))<=render_distance){
		node t_dist=eye_next(cur_position,direction);
		ll axis=select_axis(t_dist);
		if(axis<=0||axis>FACE_NUM)break;
		cube_relative=cur_position-cube_num;
		
		hit_cube hc;
		hc.cube_num=cube_num;
		hc.p0=cube_relative;
		hc.v=direction;
		calc_ray(hc);
		
		
		if(hc.is_hit){
			return {hc.inlight_direction,hc.cube_num};
		}
		advance_ray(axis,direction,t_dist,cur_position,cube_num);
	}
	return {0,{0,0,0}};
}
pair<ll,point>mirror_eyegogogo(node tmp_position,node direction,ll mr_times){//镜中手看到的方块 
	node cur_position=tmp_position;
	if(mr_times==0)return{0,{0,0,0}};
	ll axis;
	node cube_relative;
	node t_dist;
	point cube_num=to_point(tmp_position);

	while(manhattan_dis(to_point(cur_position),to_point(tmp_position))<=render_distance){
		t_dist=eye_next(cur_position,direction);
		axis=select_axis(t_dist);
		if(axis<=0||axis>FACE_NUM)break;

		cube_relative=cur_position-cube_num;
		
		hit_cube hc;
		hc.cube_num=cube_num;
		hc.p0=cube_relative;
		hc.v=direction;
		calc_ray(hc);
		
		
		if(hc.is_hit){
			if(hc.is_mirror==0){
				return {hc.inlight_direction,hc.cube_num};
			}
			else{
				return mirror_eyegogogo(hc.cube_num+hc.p,hc.v_next,mr_times-1);
			}
		}
		advance_ray(axis,direction,t_dist,cur_position,cube_num);
	}
	return{0,{0,0,0}};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////世界更改函数 

void add_cube(pair<ll,point>pid,ll put_cube){//对应位置添加方块 
	if(put_cube<0||put_cube>kind_of_cube)return;
	ll inlight_direction=pid.first;
	point cube_num=pid.second;
	if(inlight_direction<=0)return;
	if(inlight_direction!=CUBE_SELF){
		ll diff[3]={0,0,0}; 
		diff[(inlight_direction-1)/2]+=(inlight_direction%2)*(-2)+1;
		cube_num+=(point){diff[0],diff[1],diff[2]};
	}

	if(get_block(0,cube_num)==0){//原来是空气 
		if(put_cube==0)return;
		cube[0][cube_num]=put_cube;//放的方块如果不是空气 
		for(ll i=1;i<=MAX_LOD;i++){
			cube[i][to_point(cube_num*1.0/pow_2[i])]++;
		}
	}
	//原来不是空气 
	else if(put_cube==0){//放的方块是空气 
		cube[0][cube_num]=0;//
		for(ll i=1;i<=MAX_LOD;i++){
			--cube[i][to_point(cube_num*1.0/pow_2[i])];//
		}
	}
	else{
		cube[0][cube_num]=put_cube;
	}
	rot_state.erase(cube_num);
	return;
}
void combo_delete_temp(point cube_num){
	stack<point>stk;
	stk.push(cube_num);
	while(stk.size()){
		point x=stk.top();
		stk.pop();
		if(get_block(0,x)==rev_idcube["temp"]){
			cube[0][x]=0;//
			rot_state.erase(x);
			for(ll i=1;i<=MAX_LOD;i++){
				--cube[i][to_point(x*1.0/pow_2[i])];
			}
			if(get_block(0,x+(point){0,0,1})==rev_idcube["temp"])stk.push(x+(point){0,0,1});
			if(get_block(0,x-(point){0,0,1})==rev_idcube["temp"])stk.push(x-(point){0,0,1});
			if(get_block(0,x+(point){0,1,0})==rev_idcube["temp"])stk.push(x+(point){0,1,0});
			if(get_block(0,x-(point){0,1,0})==rev_idcube["temp"])stk.push(x-(point){0,1,0});
			if(get_block(0,x+(point){1,0,0})==rev_idcube["temp"])stk.push(x+(point){1,0,0});
			if(get_block(0,x-(point){1,0,0})==rev_idcube["temp"])stk.push(x-(point){1,0,0});
		}
	}
	return;
}
void delete_cube(pair<ll,point>pid){//删除对应方块 
	ll inlight_direction=pid.first;
	point cube_num=pid.second;
	if(get_block(0,cube_num)==0)return; 
	if(inlight_direction!=0){
		if(get_block(0,cube_num)==rev_idcube["temp"]){
			combo_delete_temp(pid.second);
			return;
		}
		cube[0][cube_num]=0;//
		for(ll i=1;i<=MAX_LOD;i++){
			--cube[i][to_point(cube_num*1.0/pow_2[i])];
		}
		rot_state.erase(cube_num);
	}
	return;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////快捷键函数

void display_shortcut(ll pos){
	system("cls");
	for(ll i=1;i<=kind_of_cube;i++){
		cout<<i<<": "<<idcube[i]<<" ";
		if(cube_to_shortcut[i]!=0){
			gotoxy(18,i-1);
			cout<<"->"<<cube_to_shortcut[i];
		}
		cout<<endl;
	}
	gotoxy(26,pos-1);
	cout<<"<<";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////通知添加函数

void add_info(string message){
	info=message;
	info_time=getTimeOfDayMicros();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////信息添加函数

string format_status_line(){
	string res;
	res+="\x1b[0;0H";
	if(is_color)res+="\x1b[48;2;0;0;0m\x1b[38;2;255;255;255m";
	char temp[CHAR_MAXNUM];
	snprintf(temp,sizeof(temp),"Pos: %.3f %.3f %.3f (%lld %lld %lld) |Angle: %.2f %.2f  |View_r: %.2f  |Now cube: %lld, %s  |Screen size: %lld*%lld | Cube rotation: %lld,%lld | See cube: (%.0f,%.0f,%.0f) | Info: %s 									  \n|FPS: ",position.x,position.y,position.z,(ll)floor(position.x),(ll)floor(position.y),(ll)floor(position.z),view_a,view_b,view_r,now_cube,idcube[shortcut_key[now_cube]].c_str(),resolution_x[resolution],resolution_y[resolution],((eyegogogo(eye_dir).first!=0)?get_rotate(eyegogogo(eye_dir).second)/VERTEX_NUM:-1),((eyegogogo(eye_dir).first!=0)?get_rotate(eyegogogo(eye_dir).second)%VERTEX_NUM:-1),((eyegogogo(eye_dir).first!=0)?eyegogogo(eye_dir).second.x:NAN),((eyegogogo(eye_dir).first!=0)?eyegogogo(eye_dir).second.y:NAN),((eyegogogo(eye_dir).first!=0)?eyegogogo(eye_dir).second.z:NAN),info.c_str());
	res+=temp;
	ll start=max(0ll,(ll)recent_FPS.size()-15ll);
	for(ll i=start;i<(ll)recent_FPS.size();++i){
		char fps[CHAR_MAXNUM];
		snprintf(fps,sizeof(fps),"%.5lf ",recent_FPS[i]);
		res+=fps;
	}
	res+="\n| ";
	for(ll i=1;i<=9;i++){
		char shortcut[CHAR_MAXNUM];
		snprintf(shortcut,sizeof(shortcut),"%lld:%s | ",i,idcube[shortcut_key[i]].c_str());
		res+=shortcut;
	}
	if(recent_FPS.size()>15)recent_FPS.erase(recent_FPS.begin(),recent_FPS.begin()+(recent_FPS.size()-15));
	res+="\nBlock_count: ";
	res+=to_string(cube[0].size());
	if(is_color)res+="\x1b[0m";
	
	return res;
}
void add_center_cross(){//添加白色十字 
	ll center_x=resolution_x[resolution]/2;
	ll center_y=resolution_y[resolution]/2;
	for(ll x=center_x-2;x<=center_x+2;++x){
		if(x>=0&&x<=resolution_x[resolution]){
			ll idx=center_y*(resolution_x[resolution]+1)+x;
			if(idx>=0&&idx<current_frame.size())current_frame[idx]={255,255,255};
		}
	}
	for(ll y=center_y-1;y<=center_y+1;++y){
		if(y>=0&&y<=resolution_y[resolution]){
			ll idx=y*(resolution_x[resolution]+1)+center_x;
			if(idx>=0&&idx<current_frame.size())current_frame[idx]={255,255,255};
		}
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////渲染函数

void calculate_whole(){ 
	if(current_frame.size()!=(resolution_x[resolution]+1)*(resolution_y[resolution]+1)) {
		current_frame.assign((resolution_x[resolution]+1)*(resolution_y[resolution]+1),0);
		prev_screen_color.assign((resolution_x[resolution]+1)*(resolution_y[resolution]+1),POINT_INVALID_LL);
	}
	else current_frame.assign((resolution_x[resolution]+1)*(resolution_y[resolution]+1),0);
	init();
	
	
	#ifdef USE_OPENCL
    if (g_gpu.available) {
        static bool worldChanged = true;
        if (worldChanged) {
            g_gpu.uploadWorld(cube, rot_state, all_cube_color,
                              matrix_T_inv, rot_sp_cube, position,
                              render_distance);
            worldChanged = false;
        }
        g_gpu.dispatchRender(eye_dir, screen_x_dir, screen_y_dir,
                             eye_act, eye_xydir,
                             view_a, view_b, view_r,
                             is_round, resolution_x[resolution], resolution_y[resolution],
                             mirror_times, mirror_reduction,
                             current_frame);
        image_s = "";  // GPU 暂不生成字符画，可后续补充
        return;
    }
	#endif
	
	
	current_frame.assign((resolution_x[resolution]+1)*(resolution_y[resolution]+1),0);
	image_s="";
	for(ll j=resolution_y[resolution];j>=0;--j){
		for(ll i=0;i<=resolution_x[resolution];++i){
			ll idx=(resolution_y[resolution]-j)*(resolution_x[resolution]+1)+i;
			node eye_to_pixel;
			if(is_round)eye_to_pixel=eye_act*(sinn(i*1.0/resolution_x[resolution]*360+90)*sinn(j*1.0/resolution_y[resolution]*180+180))+screen_x_dir*(sinn(i*1.0/resolution_x[resolution]*360)*sinn(j*1.0/resolution_y[resolution]*180+180))+screen_y_dir*coss(j*1.0/resolution_y[resolution]*180+180);
			else eye_to_pixel=eye_act+(screen_x_dir*screen_x_wid*(i*1.0/resolution_x[resolution]*2-1))+(screen_y_dir*screen_y_wid*(j*1.0/resolution_y[resolution]*2-1));
			eye_to_pixel=round_self(eye_to_pixel);
			eye_to_pixel=normalize(eye_to_pixel);
			gogogo(position,eye_to_pixel,mirror_times,idx);
			if(!is_color&&j==resolution_y[resolution]/2&&i==resolution_x[resolution]/2){
				if(image_s.size()>0){
					ll pos=(resolution_y[resolution]-j)*(resolution_x[resolution]+2)+i;
					if(pos<image_s.size()){
						image_s[pos]='+';
					}
				}
			} 
		}
		image_s+='\n';
	}
}
void append_color_run(ll x,ll y,ll length,point color){//缓冲区添加颜色信息
	ansi_buffer+="\x1b["+to_string(y+1)+";"+to_string(x+1)+"H\x1b[48;2;"+to_string(color.x)+";"+to_string(color.y)+";"+to_string(color.z)+"m";
	ansi_buffer.append(length,' ');
	ansi_buffer+="\x1b[0m";
}
void optimized_render(){//渲染输出 
	if(!is_color){
		cout <<format_status_line()<<"\n";
		if(first_frame||force_redraw||image_s!=prev_image_s){
			if(force_redraw)system("cls");
			gotoxy(0,3);
			cout<<image_s;
			first_frame=false;
			force_redraw=false;
			prev_image_s=image_s;
			prev_screen_color=current_frame;
		}
		return;
	}
	add_center_cross();
	ansi_buffer.clear();
	ansi_buffer.reserve((resolution_x[resolution]+1)*(resolution_y[resolution]+1+1)*20);
	if(first_frame||force_redraw){
		for(ll y=0;y<=resolution_y[resolution];++y){
			for(ll x=0;x<=resolution_x[resolution];++x){
				ll idx=y*(resolution_x[resolution]+1)+x;
				point color_val=current_frame[idx];
				append_color_run(x,y,1,color_val);
			}
		}
		ansi_buffer+=format_status_line();
		system("cls");
		cout<<ansi_buffer;
		first_frame=false;
		force_redraw=false;
		prev_screen_color=current_frame;
		return;
	}
	bool need_update=false;
	for(ll y=0;y<=resolution_y[resolution];++y){
		ll start_x=INVALID_LL;
		point current_color=POINT_INVALID_LL;
		ll run_length=0;
		for(ll x=0;x<=resolution_x[resolution];++x){
			ll idx=y*(resolution_x[resolution]+1)+x;
			point prev_color=prev_screen_color[idx];
			point new_color=current_frame[idx];
			if(prev_color!=new_color){
				need_update=true;
				if(current_color!=POINT_INVALID_LL&&new_color!=current_color){
					append_color_run(start_x,y,run_length,current_color);
					start_x=x;
					current_color=new_color;
					run_length=1;
				}
				else if(current_color==POINT_INVALID_LL){
					start_x=x;
					current_color=new_color;
					run_length=1;
				}
				else ++run_length;
			}
			else if(current_color!=POINT_INVALID_LL){
				append_color_run(start_x,y,run_length,current_color);
				current_color=POINT_INVALID_LL;
				run_length=0;
			}
		}
		if(current_color!=POINT_INVALID_LL)append_color_run(start_x,y,run_length,current_color);
	}
	if(need_update){
		ansi_buffer+=format_status_line();
		cout<<ansi_buffer;
		prev_screen_color=current_frame;
	}
	else{
		string status_buffer;
		status_buffer+=format_status_line();
		cout<<status_buffer;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////保存函数

void touch_c(ll nt){//保存截图 
	FILE* fp;
	string adr="images/"+to_string(nt)+".txt";
	fp=fopen(adr.c_str(),"w");
	if(fp==NULL)return;
	fprintf(fp,"%s",image_s.c_str());
	fclose(fp);
	add_info(to_string(nt)+" image is saved");
	if(is_color){
		string adr_bmp="images/"+to_string(nt)+".bmp";
		save_frame_as_bmp(adr_bmp.c_str(),resolution_x[resolution]+1,resolution_y[resolution]+1,current_frame);
		add_info(to_string(nt)+" image of char and color is saved");
	}
	return;
}
void touch_k(ll nt){//保存存档 
	touch_c(nt);
	string adr="saves/"+to_string(nt)+".txt";
	FILE *fp=fopen(adr.c_str(),"w");
	if(fp==NULL)return;
	map<point,ll>::iterator it;
	map<point,point>chunk_to_one_air;
	map<point,point>::iterator it2;
	ll cubesize=0;
	it=cube[0].begin();
	while(it!=cube[0].end()){
		if(idcube[(*it).second]!="")++cubesize;
		
		else if(chunk_to_one_air.find(to_point((*it).first*1.0/pow_2[MAX_LOD]))==chunk_to_one_air.end()){
			cubesize++;
			chunk_to_one_air[to_point((*it).first*1.0/pow_2[MAX_LOD])]=(*it).first;
		}
		++it;
	}
	fprintf(fp,"%lld\n",cubesize);
	it=cube[0].begin();
	while(it!=cube[0].end()){
		if(idcube[(*it).second]!=""){
			fprintf(fp,"%lld %lld %lld   ",(*it).first.x,(*it).first.y,(*it).first.z);
			fprintf(fp,"%s\n",idcube[(*it).second].c_str());
		}
		++it;
	}
	
	it2=chunk_to_one_air.begin();
	while(it2!=chunk_to_one_air.end()){
		fprintf(fp,"%lld %lld %lld   ",(*it2).second.x,(*it2).second.y,(*it2).second.z);
		fprintf(fp,"0\n");
		++it2;
	}
	
	
	ll rotatesize=0;
	it=rot_state.begin();
	while(it!=rot_state.end()){
		if((*it).second)++rotatesize;
		++it;
	}
	fprintf(fp,"\n%lld\n",rotatesize);
	it=rot_state.begin();
	while(it!=rot_state.end()){
		if((*it).second){
			fprintf(fp,"%lld %lld %lld   ",(*it).first.x,(*it).first.y,(*it).first.z);
			fprintf(fp,"%lld\n",(*it).second);
		}
		++it;
	}
	
	fclose(fp);
	change=0;
	add_info(to_string(nt)+" save is saved");
	return;   
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////读存档函数

string getPreviewImage(const string& filename){//得到存档预览 
	string filepath="images/"+filename;
	ifstream file(filepath,ios::binary|ios::ate);
	if(!file.is_open())return "";
	streamsize size=file.tellg();
	if(size<=0)return "";
	file.seekg(0,ios::beg);
	string content;
	content.resize(static_cast<size_t>(size));
	if(file.read(const_cast<char*>(content.data()),size))return content;
	return "";
}
void column_print(const string&content,ll column){
	istringstream stream(content);
	string line;
	while(getline(stream,line))cout<<"\x1b["<<column<<"G"<<line<<"\n";
}
void displaySaveList(const vector<string>&fileNames,ll pos){
    system("cls");
    for(size_t i=0;i<fileNames.size();++i){
        cout<<fileNames[i]<<' ';
        if((ll)i==pos)cout<<"\x1b[s"<<"<<";
        cout<<endl;
    }
    cout<<"\nexit ";
    if(pos==(ll)fileNames.size())cout<<"\x1b[s"<<"<<";
    cout<<"\n\n";
    if(pos>=0&&pos<(ll)fileNames.size()){
        ll selectedRow=pos+1;
        cout<<"\x1b["<<selectedRow<<";25H";
        string preview=getPreviewImage(fileNames[pos]);
        column_print(downsample(preview,80,200),25);
    }
    cout<<"\x1b[u";
    ScrollWindowToCursor();
}
void init_chunk(ll lod,point pos){
	cube[lod][pos];
	if(lod>0){
		for(ll i=0;i<VERTEX_NUM;i++){
			init_chunk(lod-1,(pos*2ll)+(point){((i>>2ll)&1ll),((i>>1ll)&1ll),((i>>0ll)&1ll)});
		}
	}
	return;
}
void loadSave(const string& filename){//加载存档 
	FILE*fp=fopen(("saves/"+filename).c_str(),"r");
	if(fp==NULL)return;
	ll cubesize=0;
	fscanf(fp,"%lld",&cubesize);
	if(cubesize==0){
		fclose(fp);
		return; 
	}
	for(ll i=0;i<=MAX_LOD;i++)cube[i].clear();
	
	map<point,bool>to_init_chunk;
	//不要写成set，有神秘问题 
	for(ll i=0;i<cubesize;++i){
		point cube_num={0,0,0};
		char cubestyle[CHAR_MAXNUM];
		fscanf(fp,"%lld %lld %lld",&cube_num.x,&cube_num.y,&cube_num.z);
		fscanf(fp,"%s",cubestyle);
		string style(cubestyle);
		bool is_number=true;
		for(char c:style)if(c<'0'||c>'9'){is_number=false;break;}
		ll block_kind=0;
		if(is_number)block_kind=stoi(style);
		else block_kind=rev_idcube[style];
		
		cube[0][cube_num]=block_kind;
		to_init_chunk[to_point(cube_num*1.0/pow_2[MAX_LOD])]=1;
		
		for(ll i=1;i<=MAX_LOD;i++){
			if(block_kind>0)cube[i][to_point(cube_num*1.0/pow_2[i])]++;
		}
	}
	map<point,bool>::iterator it=to_init_chunk.begin();
	while(it!=to_init_chunk.end()){
		if((*it).second)init_chunk(MAX_LOD,(*it).first);
		it++;
	}
	
	ll rotatesize=0;
	fscanf(fp,"%lld",&rotatesize);
	if(rotatesize==0){
		fclose(fp);
		return;
	}
	rot_state.clear();
	for(ll i=0;i<rotatesize;++i){
		point cube_num={0,0,0};
		ll rotatestyle=0;
		fscanf(fp,"%lld %lld %lld",&cube_num.x,&cube_num.y,&cube_num.z);
		fscanf(fp,"%lld",&rotatestyle);
		if(rotatestyle)rot_state[cube_num]=rotatestyle;
	}
	fclose(fp);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////主函数 

signed main(){
	over_all_init();
	
	#ifdef USE_OPENCL
	g_gpu.init();                         // 初始化 GPU
	if (g_gpu.available) {
	    add_info("OpenCL GPU available");
	}else {
	    add_info("OpenCL GPU NOT available, falling back to CPU");
	}
	#endif
	
	while(1){
		if(getTimeOfDayMicros()-info_time>INFO_TIMEOUT)info="";
		HideCursor();
		ll start_time=getTimeOfDayMicros();
		calculate_whole();
		optimized_render();
		ll end_time=getTimeOfDayMicros();
		now_FPS=SECOND_TO_MICROSECOND/(end_time-start_time);
		recent_FPS.push_back(now_FPS);
		if(_kbhit()){
			key_ch=_getch();
			keyboard();
		}
		if(is_mouse)mouse();
	}
	return 0;
}



