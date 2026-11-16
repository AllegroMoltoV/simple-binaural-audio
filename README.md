# Simple Binaural

DAW 向けの立体音響 (バイノーラル) のオーディオプラグインです。入力の音を、聞き手のまわりの方向・高さ・距離に置き、ヘッドホンで聞いたときに、その位置から聞こえるようにします。Windows x64 と Linux x86_64 向けに、VST® 3 と CLAP の形式で配布します。

A binaural spatializer for DAWs. It places the input around the listener by direction, elevation, distance and room, for headphones. VST® 3 and CLAP formats for Windows x64 and Linux x86_64.

音のプラグイン倉庫くんの[掲載ページ](https://audio-soko-kun.allegromoltov.jp/plugins/allegromoltov/simple-binaural-audio)から、検査を通った版を入手できます。 / Listed on [Audio Plugin Soko-kun](https://audio-soko-kun.allegromoltov.jp/plugins/allegromoltov/simple-binaural-audio?lang=en), which links only to releases that passed its checks.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="docs/images/VST_Compatible_Logo_Steinberg_negative.png">
  <img src="docs/images/VST_Compatible_Logo_Steinberg.png" alt="VST Compatible" width="120">
</picture>

## 機能

信号処理は、Google が公開した [Resonance Audio](https://github.com/resonance-audio/resonance-audio) を使います。音源を Ambisonics に変換し、ダミーヘッドで測った頭部伝達関数 (SADIE のデータベース) でヘッドホン向けの 2 チャンネルにします。

| パラメーター | 範囲 | 既定値 | 内容 |
| --- | --- | --- | --- |
| Azimuth | -180〜180 度 | 0 度 | 水平の方向。0 度が正面、+90 度が右、-90 度が左です |
| Elevation | -90〜90 度 | 0 度 | 高さ。+90 度が真上です |
| Distance | 0.25〜10 m | 1 m | 距離。遠いほど小さく、近いほど耳元の近さの効果が付きます |
| Room | 0〜100 % | 0 % | 部屋の反射と残響の量。中くらいの部屋を固定で使います |
| Quality | Low、Medium、High | High | 1 次・2 次・3 次の Ambisonics。高いほど方向の再現が細かく、処理が重くなります |

- ステレオの入力は、左右を平均して 1 つの音源にします。出力はヘッドホン向けのステレオです。
- レイテンシーは 256 サンプルです (48 kHz で約 5.3 ms)。DAW に報告するため、DAW の遅延補正で前後の位置がそろいます。
- サンプルレートは 22.05 kHz から 384 kHz までの一般的な値 (44.1 kHz、48 kHz、88.2 kHz、96 kHz、176.4 kHz、192 kHz など) で立体化します。Resonance Audio が扱えない値では、立体化せずに、左右を平均した原音を同じ 256 サンプル遅らせて出します。
- 処理の重さは、手元の Windows の計測で、48 kHz の High で実時間の約 0.7 % です。
- 通信しません。音声やデータを外部へ送りません。

ヘッドホンで聞くための処理です。スピーカーで聞くと、方向の効果はほとんど得られません。

## 動作環境

- Windows 10 / 11 x64
- Linux x86_64
- VST 3 または CLAP に対応した DAW

macOS 向けには配布しません。macOS 向けの配布には、Apple の Developer ID の署名と公証が要るためです。

Linux 向けの版は、CI の pluginval と clap-validator で検証しています。Linux の DAW での動作は確かめていません。

## インストール

1. DAW を終了します。
2. [GitHub Releases](https://github.com/AllegroMoltoV/simple-binaural-audio/releases) から、環境に合う zip をダウンロードして展開します。Windows では `simple-binaural-<版>-windows-x64.zip`、Linux では `simple-binaural-<版>-linux-x86_64.zip` です。
3. Windows では、`VST3\Simple Binaural.vst3` フォルダーを、次のどちらかへコピーします。
   - 自分だけが使う場合: `%LOCALAPPDATA%\Programs\Common\VST3\`
   - すべてのユーザーが使う場合: `C:\Program Files\Common Files\VST3\`。管理者の権限が必要です。
4. Windows の CLAP に対応した DAW では、`CLAP\Simple Binaural.clap` を `%LOCALAPPDATA%\Programs\Common\CLAP\` か `C:\Program Files\Common Files\CLAP\` へコピーします。
5. Linux では、`VST3/Simple Binaural.vst3` フォルダーを `~/.vst3/` へ、`CLAP/Simple Binaural.clap` を `~/.clap/` へコピーします。
6. DAW を起動し、必要ならプラグインを再スキャンします。

インストーラーはありません。

## ダウンロードしたファイルの確かめ方

リリースのファイルには、GitHub Actions でビルドしたことを示す証明 (artifact attestation) を付けています。[GitHub CLI](https://cli.github.com/) で確かめられます。

```sh
gh attestation verify simple-binaural-<版>-<環境>.zip --repo AllegroMoltoV/simple-binaural-audio
```

リリースは不変 (immutable release) に設定しており、公開後にファイルを差し替えられません。

## ビルド

CMake 3.21 以降と C++17 のコンパイラーが必要です。Windows では Visual Studio 2022 以降を使い、CMake はインストールされている中で最も新しい Visual Studio を選びます。Linux では GCC を使います。

```sh
# Windows
cmake -B build -A x64
# Linux
cmake -B build -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release
ctest --test-dir build -C Release
```

Resonance Audio は、上流のリポジトリが 2023 年に更新を止めています。このプラグインは、Qt が自身の [qtmultimedia](https://github.com/qt/qtmultimedia/tree/5d5bb219c326e8ea796643e405ab6ae0a05483e6/src/3rdparty) に取り込んで修正を当てている版を、コミットと SHA-256 を固定して構成の時点で取得します。Eigen と pffft も、Qt がそこで使っている版 (Eigen 5.0.1、pffft 02fe7715) を使います。Qt 自体はビルドしません。

単体テスト `tests/binaural-test.cpp` では、左右の音量差と到達時間の差 (右の音源で右が約 10 dB 大きく、約 0.77 ms 早い)、報告したレイテンシー、DAW のブロック長によらない出力、扱えないサンプルレートで落ちずに原音を通すこと、方向を急に変えたときのクリックの無さを確かめます。CI では、単体テストに加えて、[pluginval](https://github.com/Tracktion/pluginval) と [clap-validator](https://github.com/free-audio/clap-validator) で検証します。pluginval は VST 3 SDK の validator を内蔵しており、あわせて実行します。

## ライセンス

MIT License です。[LICENSE](LICENSE) を参照してください。

`docs/images/` の VST Compatible ロゴの画像は Steinberg Media Technologies GmbH の商標で、MIT License の対象外です。

配布するファイルに含む第三者のソフトウェアとデータ、そのライセンスは次のとおりです。いずれも配布する zip の `THIRD_PARTY_LICENSES` にライセンス文を同梱しています。

| ソフトウェア・データ | 用途 | ライセンス |
| --- | --- | --- |
| [Resonance Audio](https://github.com/resonance-audio/resonance-audio) (Qt のパッチ済みの版) | バイノーラル化 | Apache-2.0 |
| SADIE HRTF Database (University of York, 2016) | 頭部伝達関数のデータ。Resonance Audio に同梱 | Apache-2.0 |
| [Eigen](https://gitlab.com/libeigen/eigen) 5.0.1 | Resonance Audio の依存。MPL-2.0 の部分だけを使います | MPL-2.0 |
| [pffft](https://bitbucket.org/jpommier/pffft) | Resonance Audio の依存 (FFT) | BSD 系 (FFTPACK) |
| [CLAP](https://github.com/free-audio/clap) | CLAP の形式 | MIT |
| [clap-wrapper](https://github.com/free-audio/clap-wrapper) | CLAP から VST 3 の生成 | MIT |
| [{fmt}](https://github.com/fmtlib/fmt) | clap-wrapper の依存 | MIT |
| [VST 3 SDK](https://github.com/steinbergmedia/vst3sdk) | VST 3 の形式 | MIT |

Eigen は変更せずに使っています。使ったソースは、上の qtmultimedia のコミットの `src/3rdparty/eigen` にあります。

頭部伝達関数は、University of York の SADIE プロジェクトの SADIE HRTF Database (2016) を使っています。

VST is a registered trademark of Steinberg Media Technologies GmbH.

## 更新履歴

### 1.0.0

- 最初の版です。
