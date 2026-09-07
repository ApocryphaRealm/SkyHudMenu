# -*- coding: utf-8 -*-
"""gen-translations.py - builds the eleven SkyHudMenu_<language>.txt files.

The English key list is extracted from the PATCHED source so it can never drift from the code:
  * every literal `strings::TR("KEY", "text")` in source/UI.cpp (regex) - the mod's own fixed
    strings (buttons, headers, status lines, the "X"/"Y" position-input labels);
  * every distinct label/option text in source/HudElements.cpp's table (Element.name,
    PosPair.label, ToggleField.label, DropdownField.label and its option display text),
    transcribed here once and run through the SAME DeriveKey algorithm UI.cpp uses
    (source/UI.cpp's DeriveKey()) so the runtime key and this generator's key are byte-identical.
    Two table entries with the same text (every plain "Position" tab, the shared alignment/hour
    tables) collapse onto one key on purpose - one translation serves every use.

Writes REPO/dist/Interface/Translations/SkyHudMenu_<language>.txt for english + the owner's ten
languages (UTF-16LE with a BOM, one "$key<TAB>text" per line, literal "\\n" for an embedded line
break, CRLF records - the SKSE/SkyUI shape AMF's own Strings.cpp reads).

Run: `python tools/gen-translations.py` from the repo root or anywhere (paths are relative to
this script's grandparent directory).
"""
import io
import os
import re

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LANGS = ["english", "japanese", "korean", "chinese", "russian", "german", "french", "spanish", "italian", "polish", "czech"]

TR_RE = re.compile(r'strings::TR\(\s*"((?:[^"\\]|\\.)+)"\s*,\s*"((?:[^"\\]|\\.)*)"\s*\)')


def unescape(s):
    return s.encode("latin-1", "backslashreplace").decode("unicode_escape") if "\\" in s else s


def derive_key(prefix, text):
    """Byte-identical to UI.cpp's DeriveKey(): each run of ASCII alnum chars is one word,
    capitalised on its first character; anything else is a word break and contributes nothing."""
    out = [prefix]
    start_word = True
    for ch in text:
        if ch.isalnum() and ord(ch) < 128:
            out.append(ch.upper() if start_word else ch)
            start_word = False
        else:
            start_word = True
    return "".join(out)


# ------------------------------------------------------------------------------------------------
# 1) Fixed keys: extracted from the patched source/UI.cpp.
# ------------------------------------------------------------------------------------------------
def read_fixed_keys():
    path = os.path.join(REPO, "source", "UI.cpp")
    src = io.open(path, "r", encoding="utf-8").read()
    keys = {}
    for m in TR_RE.finditer(src):
        key, text = unescape(m.group(1)), unescape(m.group(2))
        if key in keys and keys[key] != text:
            raise RuntimeError(f"duplicate fixed key {key!r} with two different English texts")
        keys[key] = text
    return keys


FIXED_TRANSLATIONS = {
    "SHM_NoConfig": {
        "japanese": "skyhud.txt が見つかりませんでした。このメニューはSkyHUDの設定を編集するため、SkyHUD(またはskyhud.txtを提供するプリセット)がインストールされている必要があります。",
        "korean": "skyhud.txt 파일을 찾을 수 없습니다. 이 메뉴는 SkyHUD의 설정을 편집하므로 SkyHUD(또는 skyhud.txt를 제공하는 프리셋)가 설치되어 있어야 합니다.",
        "chinese": "未找到 skyhud.txt。此菜单用于编辑 SkyHUD 的设置,因此必须安装 SkyHUD(或提供 skyhud.txt 的预设)。",
        "russian": "Файл skyhud.txt не найден. Это меню редактирует настройки SkyHUD, поэтому должен быть установлен SkyHUD (или пресет, предоставляющий skyhud.txt).",
        "german": "skyhud.txt wurde nicht gefunden. Dieses Menü bearbeitet die Einstellungen von SkyHUD, daher muss SkyHUD (oder ein Preset, das skyhud.txt bereitstellt) installiert sein.",
        "french": "skyhud.txt est introuvable. Ce menu modifie les paramètres de SkyHUD ; SkyHUD (ou un préréglage fournissant skyhud.txt) doit donc être installé.",
        "spanish": "No se encontró skyhud.txt. Este menú edita los ajustes de SkyHUD, por lo que SkyHUD (o un preset que proporcione skyhud.txt) debe estar instalado.",
        "italian": "skyhud.txt non è stato trovato. Questo menu modifica le impostazioni di SkyHUD, quindi SkyHUD (o un preset che fornisce skyhud.txt) deve essere installato.",
        "polish": "Nie znaleziono skyhud.txt. To menu edytuje ustawienia SkyHUD, więc SkyHUD (lub zestaw ustawień dostarczający skyhud.txt) musi być zainstalowany.",
        "czech": "Soubor skyhud.txt nebyl nalezen. Tato nabídka upravuje nastavení SkyHUD, takže musí být nainstalován SkyHUD (nebo předvolba poskytující skyhud.txt).",
    },
    "SHM_EditingPath": {
        "japanese": "編集中: %s", "korean": "편집 중: %s", "chinese": "正在编辑:%s", "russian": "Редактирование: %s",
        "german": "Bearbeite %s", "french": "Modification de %s", "spanish": "Editando %s", "italian": "Modifica di %s",
        "polish": "Edytowanie %s", "czech": "Úprava %s",
    },
    "SHM_TabHint": {
        "japanese": "各タブは1つのHUD要素です。保存するとskyhud.txtに書き込まれます。SkyHUDはゲーム開始時にこれを読み込むため、保存した変更は再起動後に表示されます。",
        "korean": "각 탭은 하나의 HUD 요소입니다. 저장하면 skyhud.txt에 기록됩니다. SkyHUD는 게임 시작 시 이를 읽으므로 저장한 변경 사항은 재시작 후에 표시됩니다.",
        "chinese": "每个标签对应一个 HUD 元素。保存会写入 skyhud.txt;SkyHUD 在游戏启动时读取该文件,因此保存的更改需要重启后才会显示。",
        "russian": "Каждая вкладка — это один элемент HUD. Сохранение записывает skyhud.txt; SkyHUD считывает его при запуске игры, поэтому сохранённое изменение появится после перезапуска.",
        "german": "Jeder Tab ist ein HUD-Element. Speichern schreibt skyhud.txt; SkyHUD liest sie beim Spielstart, daher wird eine gespeicherte Änderung erst nach einem Neustart angezeigt.",
        "french": "Chaque onglet correspond à un élément du HUD. Enregistrer écrit dans skyhud.txt ; SkyHUD le lit au démarrage du jeu, donc un changement enregistré n'apparaît qu'après un redémarrage.",
        "spanish": "Cada pestaña es un elemento del HUD. Guardar escribe skyhud.txt; SkyHUD lo lee al iniciar el juego, por lo que un cambio guardado se muestra tras reiniciar.",
        "italian": "Ogni scheda è un elemento della HUD. Salvare scrive skyhud.txt; SkyHUD lo legge all'avvio del gioco, quindi una modifica salvata viene mostrata dopo un riavvio.",
        "polish": "Każda zakładka to jeden element HUD. Zapis zapisuje plik skyhud.txt; SkyHUD odczytuje go podczas uruchamiania gry, więc zapisana zmiana pojawi się po ponownym uruchomieniu.",
        "czech": "Každá karta je jeden prvek HUD. Uložení zapíše skyhud.txt; SkyHUD jej načte při spuštění hry, takže uložená změna se zobrazí až po restartu.",
    },
    "SHM_ShowMarkers": {
        "japanese": "画面上に位置マーカーを表示", "korean": "화면에 위치 마커 표시", "chinese": "显示屏幕位置标记",
        "russian": "Показывать маркеры позиции на экране", "german": "Positionsmarker auf dem Bildschirm anzeigen",
        "french": "Afficher les repères de position à l'écran", "spanish": "Mostrar marcadores de posición en pantalla",
        "italian": "Mostra indicatori di posizione a schermo", "polish": "Pokaż znaczniki pozycji na ekranie",
        "czech": "Zobrazit značky pozice na obrazovce",
    },
    "SHM_MarkersHint": {
        "japanese": "配置されたすべての要素のゴースト表示 - メニューを閉じても表示されます",
        "korean": "배치된 모든 요소의 고스트 - 메뉴를 닫아도 표시됩니다",
        "chinese": "显示所有已定位元素的幽影 - 关闭菜单时同样可见",
        "russian": "Призраки всех позиционированных элементов — видны даже при закрытом меню",
        "german": "Umrisse aller positionierten Elemente - auch bei geschlossenem Menü sichtbar",
        "french": "Contours de chaque élément positionné - visibles même avec le menu fermé",
        "spanish": "Contornos de cada elemento posicionado - visibles incluso con el menú cerrado",
        "italian": "Contorni di ogni elemento posizionato - visibili anche a menu chiuso",
        "polish": "Zarysy każdego rozmieszczonego elementu - widoczne także po zamknięciu menu",
        "czech": "Obrysy každého umístěného prvku - viditelné i se zavřenou nabídkou",
    },
    "SHM_ShowGhost": {
        "japanese": "この要素のゴーストを画面に表示", "korean": "이 고스트를 화면에 표시", "chinese": "在屏幕上显示此幽影",
        "russian": "Показывать этот призрак на экране", "german": "Diesen Umriss auf dem Bildschirm anzeigen",
        "french": "Afficher ce repère à l'écran", "spanish": "Mostrar este contorno en pantalla",
        "italian": "Mostra questo contorno a schermo", "polish": "Pokaż ten zarys na ekranie",
        "czech": "Zobrazit tento obrys na obrazovce",
    },
    "SHM_GhostColour": {
        "japanese": "ゴーストの色", "korean": "고스트 색상", "chinese": "幽影颜色", "russian": "Цвет призрака",
        "german": "Umrissfarbe", "french": "Couleur du repère", "spanish": "Color del contorno",
        "italian": "Colore del contorno", "polish": "Kolor zarysu", "czech": "Barva obrysu",
    },
    "SHM_Locked": {
        "japanese": "ロック(SkyHUDの既定位置を使用)", "korean": "잠금(SkyHUD의 기본 위치 사용)",
        "chinese": "锁定(使用 SkyHUD 的默认位置)", "russian": "Заблокировано (использовать позицию SkyHUD по умолчанию)",
        "german": "Gesperrt (Standardposition von SkyHUD verwenden)",
        "french": "Verrouillé (utiliser la position par défaut de SkyHUD)",
        "spanish": "Bloqueado (usar la posición predeterminada de SkyHUD)",
        "italian": "Bloccato (usa la posizione predefinita di SkyHUD)",
        "polish": "Zablokowane (użyj domyślnej pozycji SkyHUD)",
        "czech": "Zamčeno (použít výchozí pozici SkyHUD)",
    },
    "SHM_Scale": {
        "japanese": "拡大率", "korean": "크기", "chinese": "缩放", "russian": "Масштаб", "german": "Größe",
        "french": "Échelle", "spanish": "Escala", "italian": "Scala", "polish": "Skala", "czech": "Měřítko",
    },
    "SHM_Position": {
        "japanese": "位置", "korean": "위치", "chinese": "位置", "russian": "Позиция", "german": "Position",
        "french": "Position", "spanish": "Posición", "italian": "Posizione", "polish": "Pozycja", "czech": "Pozice",
    },
    "SHM_Options": {
        "japanese": "オプション", "korean": "옵션", "chinese": "选项", "russian": "Опции", "german": "Optionen",
        "french": "Options", "spanish": "Opciones", "italian": "Opzioni", "polish": "Opcje", "czech": "Možnosti",
    },
    "SHM_FooterA": {
        "japanese": "SkyHUDはゲーム開始時にのみskyhud.txtを読み込むため、保存した変更は",
        "korean": "SkyHUD는 게임 시작 시에만 skyhud.txt를 읽으므로 저장한 변경 사항은",
        "chinese": "SkyHUD 仅在游戏启动时读取 skyhud.txt,因此保存的更改会在",
        "russian": "SkyHUD считывает skyhud.txt только при запуске игры, поэтому сохранённое изменение появится после",
        "german": "SkyHUD liest skyhud.txt nur beim Spielstart, daher wird eine gespeicherte Änderung nach einem",
        "french": "SkyHUD ne lit skyhud.txt qu'au démarrage du jeu, donc un changement enregistré s'affiche après un",
        "spanish": "SkyHUD solo lee skyhud.txt al iniciar el juego, por lo que un cambio guardado se muestra tras un",
        "italian": "SkyHUD legge skyhud.txt solo all'avvio del gioco, quindi una modifica salvata viene mostrata dopo un",
        "polish": "SkyHUD odczytuje skyhud.txt tylko podczas uruchamiania gry, więc zapisana zmiana pojawi się po",
        "czech": "SkyHUD čte skyhud.txt pouze při spuštění hry, takže uložená změna se zobrazí až po",
    },
    "SHM_FooterB": {
        "japanese": "再起動が必要です - マーカーは各要素が配置される位置を今すぐ示しています。",
        "korean": "재시작이 필요합니다 - 마커는 각 요소가 배치될 위치를 지금 표시합니다.",
        "chinese": "重启后生效 - 标记会立即显示每个元素最终所在的位置。",
        "russian": "перезапуске - маркеры уже сейчас показывают, где окажется каждый элемент.",
        "german": "einem Neustart angezeigt - die Marker zeigen schon jetzt, wo jedes Element landen wird.",
        "french": "redémarrage - les repères montrent déjà où chaque élément se placera.",
        "spanish": "reiniciar - los marcadores ya muestran ahora dónde quedará cada elemento.",
        "italian": "riavvio - gli indicatori mostrano già ora dove finirà ogni elemento.",
        "polish": "ponownym uruchomieniu - znaczniki już teraz pokazują, gdzie wypadnie każdy element.",
        "czech": "restartu - značky nyní ukazují, kam každý prvek dopadne.",
    },
    "SHM_SaveBtn": {
        "japanese": "保存(再起動後に反映)", "korean": "저장(재시작 후 적용)", "chinese": "保存(重启后生效)",
        "russian": "Сохранить (появится после перезапуска)", "german": "Speichern (wirkt nach einem Neustart)",
        "french": "Enregistrer (effectif après un redémarrage)", "spanish": "Guardar (se aplica tras reiniciar)",
        "italian": "Salva (effettivo dopo un riavvio)", "polish": "Zapisz (widoczne po ponownym uruchomieniu)",
        "czech": "Uložit (zobrazí se po restartu)",
    },
    "SHM_ReloadBtn": {
        "japanese": "ファイルから再読み込み", "korean": "파일에서 다시 불러오기", "chinese": "从文件重新加载",
        "russian": "Перезагрузить из файла", "german": "Aus Datei neu laden", "french": "Recharger depuis le fichier",
        "spanish": "Recargar desde el archivo", "italian": "Ricarica dal file", "polish": "Wczytaj ponownie z pliku",
        "czech": "Znovu načíst ze souboru",
    },
    "SHM_StatusSaved": {
        "japanese": "skyhud.txtに保存しました。HUDで確認するにはゲームを再起動してください。",
        "korean": "skyhud.txt에 저장했습니다. HUD에서 확인하려면 게임을 재시작하세요.",
        "chinese": "已保存到 skyhud.txt。请重启游戏以在 HUD 中查看。",
        "russian": "Сохранено в skyhud.txt. Перезапустите игру, чтобы увидеть изменения в HUD.",
        "german": "In skyhud.txt gespeichert. Starte das Spiel neu, um es im HUD zu sehen.",
        "french": "Enregistré dans skyhud.txt. Redémarrez le jeu pour le voir dans le HUD.",
        "spanish": "Guardado en skyhud.txt. Reinicia el juego para verlo en el HUD.",
        "italian": "Salvato in skyhud.txt. Riavvia il gioco per vederlo nella HUD.",
        "polish": "Zapisano do skyhud.txt. Uruchom grę ponownie, aby zobaczyć zmiany w HUD.",
        "czech": "Uloženo do skyhud.txt. Restartujte hru, abyste změnu viděli v HUD.",
    },
    "SHM_StatusSaveFail": {
        "japanese": "skyhud.txtへの書き込みに失敗しました。ログを確認してください。",
        "korean": "skyhud.txt에 쓸 수 없습니다. 로그를 확인하세요.",
        "chinese": "无法写入 skyhud.txt。请查看日志。",
        "russian": "Не удалось записать skyhud.txt. См. журнал.",
        "german": "skyhud.txt konnte nicht geschrieben werden. Siehe Log.",
        "french": "Impossible d'écrire skyhud.txt. Consultez le journal.",
        "spanish": "No se pudo escribir skyhud.txt. Consulta el registro.",
        "italian": "Impossibile scrivere skyhud.txt. Controlla il log.",
        "polish": "Nie można zapisać skyhud.txt. Sprawdź log.",
        "czech": "Nelze zapsat skyhud.txt. Podívejte se do logu.",
    },
    "SHM_StatusReloaded": {
        "japanese": "skyhud.txtをディスクから再読み込みしました。",
        "korean": "skyhud.txt를 디스크에서 다시 불러왔습니다.",
        "chinese": "已从磁盘重新加载 skyhud.txt。",
        "russian": "skyhud.txt перезагружен с диска.",
        "german": "skyhud.txt wurde von der Festplatte neu geladen.",
        "french": "skyhud.txt rechargé depuis le disque.",
        "spanish": "skyhud.txt recargado desde el disco.",
        "italian": "skyhud.txt ricaricato dal disco.",
        "polish": "Wczytano ponownie skyhud.txt z dysku.",
        "czech": "skyhud.txt bylo znovu načteno z disku.",
    },
    "SHM_X": {lang: "X" for lang in LANGS[1:]},
    "SHM_Y": {lang: "Y" for lang in LANGS[1:]},
}


# ------------------------------------------------------------------------------------------------
# 2) Table-derived keys: every distinct label/option text in source/HudElements.cpp, transcribed
#    from the table exactly (see HudElements.cpp for the source of truth this must track).
# ------------------------------------------------------------------------------------------------
TABLE_TEXTS = [
    # Element.name (tab labels)
    "Health", "Magicka", "Stamina", "Charge meters", "Compass", "Compass markers", "Crosshair",
    "Enemy health", "Stealth meter", "Subtitles", "Ammo / arrow count", "Activate prompt",
    "Location info", "Message info", "Objective text", "Level-up meter", "Shout meter",
    "Animated letters", "Clock", "General",
    # PosPair.label
    "Position", "Left", "Right", "Combined", "Button", "Name", "Info", "Bar",
    "Word wall letters", "Shout letters",
    # ToggleField.label
    "Alternative style (left-aligned)", "Combined charge meter", "Slim compass",
    "Hide the compass (Dragonborn UI)", "Hide enemy markers", "Hide location markers",
    "Hide undiscovered locations", "Hide quest markers", "Dot crosshair", "Hide enemy health bar",
    "Hide the \"Hidden/Detected\" text", "Hide the stealth meter", "Ammo display",
    "Hide the activate button icon", "Hide the level-up meter (Dragonborn UI)",
    "Separate shout meter (Dragonborn UI)", "Show clock", "Show seconds", "Show AM/PM",
    "Always show (outside HUD too)", "Sync meter fade",
    # DropdownField.label
    "Text alignment", "Hour format", "Alignment", "Persistent meters",
    # DropdownField option display text (kAlign, kHourFormat, the [General] persistent-meter list)
    "Center", "12h (3:15)", "12h leading 0 (03:15)", "24h", "24h leading 0",
    "Auto", "Always on", "Always off",
]

# Ghost-colour swatch names (HudPreview.cpp kPalette), drawn by the Ghost colour combo; prefix SHM_Col_.
COLOUR_TEXTS = ["Gold", "Red", "Green", "Blue", "Cyan", "Magenta", "Orange", "White"]
COLOUR_TRANSLATIONS = {
    "Gold": {"japanese": "金色", "korean": "금색", "chinese": "金色", "russian": "Золотой", "german": "Gold", "french": "Or", "spanish": "Dorado", "italian": "Oro", "polish": "Złoty", "czech": "Zlatá"},
    "Red": {"japanese": "赤", "korean": "빨강", "chinese": "红色", "russian": "Красный", "german": "Rot", "french": "Rouge", "spanish": "Rojo", "italian": "Rosso", "polish": "Czerwony", "czech": "Červená"},
    "Green": {"japanese": "緑", "korean": "초록", "chinese": "绿色", "russian": "Зелёный", "german": "Grün", "french": "Vert", "spanish": "Verde", "italian": "Verde", "polish": "Zielony", "czech": "Zelená"},
    "Blue": {"japanese": "青", "korean": "파랑", "chinese": "蓝色", "russian": "Синий", "german": "Blau", "french": "Bleu", "spanish": "Azul", "italian": "Blu", "polish": "Niebieski", "czech": "Modrá"},
    "Cyan": {"japanese": "シアン", "korean": "청록", "chinese": "青色", "russian": "Голубой", "german": "Cyan", "french": "Cyan", "spanish": "Cian", "italian": "Ciano", "polish": "Cyjan", "czech": "Azurová"},
    "Magenta": {"japanese": "マゼンタ", "korean": "자홍", "chinese": "洋红色", "russian": "Пурпурный", "german": "Magenta", "french": "Magenta", "spanish": "Magenta", "italian": "Magenta", "polish": "Magenta", "czech": "Purpurová"},
    "Orange": {"japanese": "オレンジ", "korean": "주황", "chinese": "橙色", "russian": "Оранжевый", "german": "Orange", "french": "Orange", "spanish": "Naranja", "italian": "Arancione", "polish": "Pomarańczowy", "czech": "Oranžová"},
    "White": {"japanese": "白", "korean": "흰색", "chinese": "白色", "russian": "Белый", "german": "Weiß", "french": "Blanc", "spanish": "Blanco", "italian": "Bianco", "polish": "Biały", "czech": "Bílá"},
}

TABLE_TRANSLATIONS = {
    "Health": {"japanese": "体力", "korean": "체력", "chinese": "生命值", "russian": "Здоровье", "german": "Gesundheit", "french": "Santé", "spanish": "Salud", "italian": "Salute", "polish": "Zdrowie", "czech": "Zdraví"},
    "Magicka": {"japanese": "マジカ", "korean": "마력", "chinese": "魔法值", "russian": "Магия", "german": "Magie", "french": "Magie", "spanish": "Magia", "italian": "Magia", "polish": "Magia", "czech": "Magie"},
    "Stamina": {"japanese": "スタミナ", "korean": "스태미나", "chinese": "耐力值", "russian": "Запас сил", "german": "Ausdauer", "french": "Endurance", "spanish": "Aguante", "italian": "Resistenza", "polish": "Wytrzymałość", "czech": "Vytrvalost"},
    "Charge meters": {"japanese": "チャージメーター", "korean": "충전 게이지", "chinese": "充能条", "russian": "Индикаторы заряда", "german": "Ladeanzeigen", "french": "Jauges de charge", "spanish": "Medidores de carga", "italian": "Indicatori di carica", "polish": "Wskaźniki naładowania", "czech": "Ukazatele nabití"},
    "Compass": {"japanese": "コンパス", "korean": "나침반", "chinese": "指南针", "russian": "Компас", "german": "Kompass", "french": "Boussole", "spanish": "Brújula", "italian": "Bussola", "polish": "Kompas", "czech": "Kompas"},
    "Compass markers": {"japanese": "コンパスマーカー", "korean": "나침반 마커", "chinese": "指南针标记", "russian": "Метки компаса", "german": "Kompassmarkierungen", "french": "Repères de la boussole", "spanish": "Marcadores de la brújula", "italian": "Indicatori della bussola", "polish": "Znaczniki kompasu", "czech": "Značky kompasu"},
    "Crosshair": {"japanese": "クロスヘア", "korean": "조준점", "chinese": "十字准心", "russian": "Перекрестие", "german": "Fadenkreuz", "french": "Réticule", "spanish": "Retícula", "italian": "Reticolo", "polish": "Celownik", "czech": "Zaměřovací kříž"},
    "Enemy health": {"japanese": "敵の体力", "korean": "적 체력", "chinese": "敌人生命值", "russian": "Здоровье врага", "german": "Gegnergesundheit", "french": "Santé de l'ennemi", "spanish": "Salud del enemigo", "italian": "Salute del nemico", "polish": "Zdrowie wroga", "czech": "Zdraví nepřítele"},
    "Stealth meter": {"japanese": "隠密メーター", "korean": "은신 게이지", "chinese": "潜行条", "russian": "Индикатор скрытности", "german": "Schleichanzeige", "french": "Jauge de discrétion", "spanish": "Medidor de sigilo", "italian": "Indicatore di furtività", "polish": "Wskaźnik skradania", "czech": "Ukazatel skrytosti"},
    "Subtitles": {"japanese": "字幕", "korean": "자막", "chinese": "字幕", "russian": "Субтитры", "german": "Untertitel", "french": "Sous-titres", "spanish": "Subtítulos", "italian": "Sottotitoli", "polish": "Napisy", "czech": "Titulky"},
    "Ammo / arrow count": {"japanese": "弾薬 / 矢の数", "korean": "화살/탄약 개수", "chinese": "弹药/箭矢数量", "russian": "Патроны/стрелы", "german": "Munition/Pfeilanzahl", "french": "Munitions / nombre de flèches", "spanish": "Munición / cantidad de flechas", "italian": "Munizioni / numero di frecce", "polish": "Amunicja / liczba strzał", "czech": "Munice / počet šipek"},
    "Activate prompt": {"japanese": "アクティベートプロンプト", "korean": "활성화 프롬프트", "chinese": "交互提示", "russian": "Подсказка взаимодействия", "german": "Interaktionshinweis", "french": "Message d'activation", "spanish": "Mensaje de activación", "italian": "Messaggio di attivazione", "polish": "Podpowiedź aktywacji", "czech": "Výzva k aktivaci"},
    "Location info": {"japanese": "場所情報", "korean": "위치 정보", "chinese": "位置信息", "russian": "Информация о местности", "german": "Standortinfo", "french": "Infos de lieu", "spanish": "Información del lugar", "italian": "Informazioni sul luogo", "polish": "Informacje o lokacji", "czech": "Informace o lokaci"},
    "Message info": {"japanese": "メッセージ情報", "korean": "메시지 정보", "chinese": "消息信息", "russian": "Информационные сообщения", "german": "Meldungsinfo", "french": "Infos de message", "spanish": "Información de mensajes", "italian": "Informazioni sui messaggi", "polish": "Informacje o wiadomościach", "czech": "Informace o zprávách"},
    "Objective text": {"japanese": "目標テキスト", "korean": "목표 텍스트", "chinese": "目标文本", "russian": "Текст цели", "german": "Zieltext", "french": "Texte d'objectif", "spanish": "Texto de objetivo", "italian": "Testo dell'obiettivo", "polish": "Tekst celu", "czech": "Text cíle"},
    "Level-up meter": {"japanese": "レベルアップメーター", "korean": "레벨업 게이지", "chinese": "升级条", "russian": "Индикатор повышения уровня", "german": "Levelaufstiegsanzeige", "french": "Jauge de montée de niveau", "spanish": "Medidor de subida de nivel", "italian": "Indicatore di salita di livello", "polish": "Wskaźnik awansu poziomu", "czech": "Ukazatel postupu na úroveň"},
    "Shout meter": {"japanese": "シャウトメーター", "korean": "외침 게이지", "chinese": "龙吼条", "russian": "Индикатор Крика", "german": "Schreianzeige", "french": "Jauge de cri", "spanish": "Medidor de grito", "italian": "Indicatore dell'Urlo", "polish": "Wskaźnik Krzyku", "czech": "Ukazatel Křiku"},
    "Animated letters": {"japanese": "アニメーション文字", "korean": "애니메이션 문자", "chinese": "动态文字", "russian": "Анимированные буквы", "german": "Animierte Buchstaben", "french": "Lettres animées", "spanish": "Letras animadas", "italian": "Lettere animate", "polish": "Animowane litery", "czech": "Animovaná písmena"},
    "Clock": {"japanese": "時計", "korean": "시계", "chinese": "时钟", "russian": "Часы", "german": "Uhr", "french": "Horloge", "spanish": "Reloj", "italian": "Orologio", "polish": "Zegar", "czech": "Hodiny"},
    "General": {"japanese": "全般", "korean": "일반", "chinese": "常规", "russian": "Общее", "german": "Allgemein", "french": "Général", "spanish": "General", "italian": "Generale", "polish": "Ogólne", "czech": "Obecné"},

    "Position": {"japanese": "位置", "korean": "위치", "chinese": "位置", "russian": "Позиция", "german": "Position", "french": "Position", "spanish": "Posición", "italian": "Posizione", "polish": "Pozycja", "czech": "Pozice"},
    "Left": {"japanese": "左", "korean": "왼쪽", "chinese": "左", "russian": "Левый", "german": "Links", "french": "Gauche", "spanish": "Izquierda", "italian": "Sinistra", "polish": "Lewy", "czech": "Levý"},
    "Right": {"japanese": "右", "korean": "오른쪽", "chinese": "右", "russian": "Правый", "german": "Rechts", "french": "Droite", "spanish": "Derecha", "italian": "Destra", "polish": "Prawy", "czech": "Pravý"},
    "Combined": {"japanese": "統合", "korean": "결합", "chinese": "合并", "russian": "Объединённый", "german": "Kombiniert", "french": "Combiné", "spanish": "Combinado", "italian": "Combinato", "polish": "Połączony", "czech": "Kombinovaný"},
    "Button": {"japanese": "ボタン", "korean": "버튼", "chinese": "按钮", "russian": "Кнопка", "german": "Schaltfläche", "french": "Bouton", "spanish": "Botón", "italian": "Pulsante", "polish": "Przycisk", "czech": "Tlačítko"},
    "Name": {"japanese": "名前", "korean": "이름", "chinese": "名称", "russian": "Название", "german": "Name", "french": "Nom", "spanish": "Nombre", "italian": "Nome", "polish": "Nazwa", "czech": "Název"},
    "Info": {"japanese": "情報", "korean": "정보", "chinese": "信息", "russian": "Информация", "german": "Info", "french": "Infos", "spanish": "Información", "italian": "Informazioni", "polish": "Informacje", "czech": "Informace"},
    "Bar": {"japanese": "バー", "korean": "바", "chinese": "进度条", "russian": "Полоса", "german": "Balken", "french": "Barre", "spanish": "Barra", "italian": "Barra", "polish": "Pasek", "czech": "Lišta"},
    "Word wall letters": {"japanese": "言葉の壁の文字", "korean": "언어의 벽 문자", "chinese": "文字之壁字符", "russian": "Буквы Слова Силы", "german": "Buchstaben der Worthauswand", "french": "Lettres du mur des mots", "spanish": "Letras del muro de palabras", "italian": "Lettere del muro delle parole", "polish": "Litery Murów Słów", "czech": "Písmena Zdi slov"},
    "Shout letters": {"japanese": "シャウトの文字", "korean": "외침 문자", "chinese": "龙吼字符", "russian": "Буквы Крика", "german": "Schreibuchstaben", "french": "Lettres du cri", "spanish": "Letras del grito", "italian": "Lettere dell'Urlo", "polish": "Litery Krzyku", "czech": "Písmena Křiku"},

    "Alternative style (left-aligned)": {"japanese": "別スタイル(左揃え)", "korean": "대체 스타일(왼쪽 정렬)", "chinese": "替代样式(左对齐)", "russian": "Альтернативный стиль (по левому краю)", "german": "Alternativer Stil (linksbündig)", "french": "Style alternatif (aligné à gauche)", "spanish": "Estilo alternativo (alineado a la izquierda)", "italian": "Stile alternativo (allineato a sinistra)", "polish": "Alternatywny styl (wyrównany do lewej)", "czech": "Alternativní styl (zarovnaný vlevo)"},
    "Combined charge meter": {"japanese": "チャージメーターを統合", "korean": "충전 게이지 결합", "chinese": "合并充能条", "russian": "Объединённый индикатор заряда", "german": "Kombinierte Ladeanzeige", "french": "Jauge de charge combinée", "spanish": "Medidor de carga combinado", "italian": "Indicatore di carica combinato", "polish": "Połączony wskaźnik naładowania", "czech": "Kombinovaný ukazatel nabití"},
    "Slim compass": {"japanese": "スリムなコンパス", "korean": "얇은 나침반", "chinese": "纤细指南针", "russian": "Тонкий компас", "german": "Schmaler Kompass", "french": "Boussole fine", "spanish": "Brújula fina", "italian": "Bussola sottile", "polish": "Wąski kompas", "czech": "Tenký kompas"},
    "Hide the compass (Dragonborn UI)": {"japanese": "コンパスを非表示(Dragonborn UI)", "korean": "나침반 숨기기(Dragonborn UI)", "chinese": "隐藏指南针(Dragonborn UI)", "russian": "Скрыть компас (Dragonborn UI)", "german": "Kompass ausblenden (Dragonborn UI)", "french": "Masquer la boussole (Dragonborn UI)", "spanish": "Ocultar la brújula (Dragonborn UI)", "italian": "Nascondi la bussola (Dragonborn UI)", "polish": "Skryj kompas (Dragonborn UI)", "czech": "Skrýt kompas (Dragonborn UI)"},
    "Hide enemy markers": {"japanese": "敵マーカーを非表示", "korean": "적 마커 숨기기", "chinese": "隐藏敌人标记", "russian": "Скрыть метки врагов", "german": "Gegnermarkierungen ausblenden", "french": "Masquer les repères d'ennemis", "spanish": "Ocultar marcadores de enemigos", "italian": "Nascondi indicatori dei nemici", "polish": "Skryj znaczniki wrogów", "czech": "Skrýt značky nepřátel"},
    "Hide location markers": {"japanese": "場所マーカーを非表示", "korean": "위치 마커 숨기기", "chinese": "隐藏地点标记", "russian": "Скрыть метки местности", "german": "Standortmarkierungen ausblenden", "french": "Masquer les repères de lieux", "spanish": "Ocultar marcadores de lugares", "italian": "Nascondi indicatori dei luoghi", "polish": "Skryj znaczniki lokacji", "czech": "Skrýt značky lokací"},
    "Hide undiscovered locations": {"japanese": "未発見の場所を非表示", "korean": "미발견 위치 숨기기", "chinese": "隐藏未发现地点", "russian": "Скрыть неоткрытые места", "german": "Unentdeckte Orte ausblenden", "french": "Masquer les lieux non découverts", "spanish": "Ocultar lugares no descubiertos", "italian": "Nascondi i luoghi non scoperti", "polish": "Skryj nieodkryte lokacje", "czech": "Skrýt neobjevené lokace"},
    "Hide quest markers": {"japanese": "クエストマーカーを非表示", "korean": "퀘스트 마커 숨기기", "chinese": "隐藏任务标记", "russian": "Скрыть метки заданий", "german": "Questmarkierungen ausblenden", "french": "Masquer les repères de quêtes", "spanish": "Ocultar marcadores de misiones", "italian": "Nascondi indicatori delle missioni", "polish": "Skryj znaczniki zadań", "czech": "Skrýt značky úkolů"},
    "Dot crosshair": {"japanese": "ドット型クロスヘア", "korean": "점 조준점", "chinese": "点状十字准心", "russian": "Точечное перекрестие", "german": "Punkt-Fadenkreuz", "french": "Réticule en point", "spanish": "Retícula de punto", "italian": "Reticolo a punto", "polish": "Celownik punktowy", "czech": "Bodový zaměřovací kříž"},
    "Hide enemy health bar": {"japanese": "敵の体力バーを非表示", "korean": "적 체력 바 숨기기", "chinese": "隐藏敌人生命条", "russian": "Скрыть полосу здоровья врага", "german": "Gegnergesundheitsleiste ausblenden", "french": "Masquer la barre de santé de l'ennemi", "spanish": "Ocultar la barra de salud del enemigo", "italian": "Nascondi la barra della salute del nemico", "polish": "Skryj pasek zdrowia wroga", "czech": "Skrýt ukazatel zdraví nepřítele"},
    "Hide the \"Hidden/Detected\" text": {"japanese": "「隠密/発見」テキストを非表示", "korean": "'은신/발견' 텍스트 숨기기", "chinese": "隐藏“隐蔽/发现”文字", "russian": "Скрыть текст «Скрыт/Обнаружен»", "german": "Text „Verborgen/Entdeckt“ ausblenden", "french": "Masquer le texte « Caché/Détecté »", "spanish": "Ocultar el texto \"Oculto/Detectado\"", "italian": "Nascondi il testo \"Nascosto/Scoperto\"", "polish": "Skryj tekst \"Skryty/Wykryty\"", "czech": "Skrýt text \"Skrytý/Odhalený\""},
    "Hide the stealth meter": {"japanese": "隠密メーターを非表示", "korean": "은신 게이지 숨기기", "chinese": "隐藏潜行条", "russian": "Скрыть индикатор скрытности", "german": "Schleichanzeige ausblenden", "french": "Masquer la jauge de discrétion", "spanish": "Ocultar el medidor de sigilo", "italian": "Nascondi l'indicatore di furtività", "polish": "Skryj wskaźnik skradania", "czech": "Skrýt ukazatel skrytosti"},
    "Ammo display": {"japanese": "弾薬表示", "korean": "탄약 표시", "chinese": "弹药显示", "russian": "Отображение патронов", "german": "Munitionsanzeige", "french": "Affichage des munitions", "spanish": "Visualización de munición", "italian": "Visualizzazione delle munizioni", "polish": "Wyświetlanie amunicji", "czech": "Zobrazení munice"},
    "Hide the activate button icon": {"japanese": "アクティベートボタンアイコンを非表示", "korean": "활성화 버튼 아이콘 숨기기", "chinese": "隐藏交互按钮图标", "russian": "Скрыть значок кнопки взаимодействия", "german": "Interaktionstastensymbol ausblenden", "french": "Masquer l'icône du bouton d'activation", "spanish": "Ocultar el icono del botón de activación", "italian": "Nascondi l'icona del tasto di attivazione", "polish": "Skryj ikonę przycisku aktywacji", "czech": "Skrýt ikonu tlačítka aktivace"},
    "Hide the level-up meter (Dragonborn UI)": {"japanese": "レベルアップメーターを非表示(Dragonborn UI)", "korean": "레벨업 게이지 숨기기(Dragonborn UI)", "chinese": "隐藏升级条(Dragonborn UI)", "russian": "Скрыть индикатор повышения уровня (Dragonborn UI)", "german": "Levelaufstiegsanzeige ausblenden (Dragonborn UI)", "french": "Masquer la jauge de montée de niveau (Dragonborn UI)", "spanish": "Ocultar el medidor de subida de nivel (Dragonborn UI)", "italian": "Nascondi l'indicatore di salita di livello (Dragonborn UI)", "polish": "Skryj wskaźnik awansu poziomu (Dragonborn UI)", "czech": "Skrýt ukazatel postupu na úroveň (Dragonborn UI)"},
    "Separate shout meter (Dragonborn UI)": {"japanese": "シャウトメーターを分離(Dragonborn UI)", "korean": "외침 게이지 분리(Dragonborn UI)", "chinese": "分离龙吼条(Dragonborn UI)", "russian": "Отдельный индикатор Крика (Dragonborn UI)", "german": "Getrennte Schreianzeige (Dragonborn UI)", "french": "Jauge de cri séparée (Dragonborn UI)", "spanish": "Medidor de grito separado (Dragonborn UI)", "italian": "Indicatore dell'Urlo separato (Dragonborn UI)", "polish": "Osobny wskaźnik Krzyku (Dragonborn UI)", "czech": "Oddělený ukazatel Křiku (Dragonborn UI)"},
    "Show clock": {"japanese": "時計を表示", "korean": "시계 표시", "chinese": "显示时钟", "russian": "Показывать часы", "german": "Uhr anzeigen", "french": "Afficher l'horloge", "spanish": "Mostrar el reloj", "italian": "Mostra l'orologio", "polish": "Pokaż zegar", "czech": "Zobrazit hodiny"},
    "Show seconds": {"japanese": "秒を表示", "korean": "초 표시", "chinese": "显示秒数", "russian": "Показывать секунды", "german": "Sekunden anzeigen", "french": "Afficher les secondes", "spanish": "Mostrar los segundos", "italian": "Mostra i secondi", "polish": "Pokaż sekundy", "czech": "Zobrazit sekundy"},
    "Show AM/PM": {"japanese": "午前/午後を表示", "korean": "오전/오후 표시", "chinese": "显示上午/下午", "russian": "Показывать AM/PM", "german": "AM/PM anzeigen", "french": "Afficher AM/PM", "spanish": "Mostrar AM/PM", "italian": "Mostra AM/PM", "polish": "Pokaż AM/PM", "czech": "Zobrazit AM/PM"},
    "Always show (outside HUD too)": {"japanese": "常に表示(HUD外でも)", "korean": "항상 표시(HUD 밖에서도)", "chinese": "始终显示(即使在 HUD 之外)", "russian": "Всегда показывать (даже вне HUD)", "german": "Immer anzeigen (auch außerhalb des HUD)", "french": "Toujours afficher (même hors HUD)", "spanish": "Mostrar siempre (incluso fuera del HUD)", "italian": "Mostra sempre (anche fuori dalla HUD)", "polish": "Pokazuj zawsze (także poza HUD)", "czech": "Vždy zobrazit (i mimo HUD)"},
    "Sync meter fade": {"japanese": "メーターのフェードを同期", "korean": "게이지 페이드 동기화", "chinese": "同步条形淡出", "russian": "Синхронизировать затухание индикаторов", "german": "Anzeigen-Ausblenden synchronisieren", "french": "Synchroniser l'estompage des jauges", "spanish": "Sincronizar el desvanecimiento de medidores", "italian": "Sincronizza la dissolvenza degli indicatori", "polish": "Synchronizuj zanikanie wskaźników", "czech": "Synchronizovat blednutí ukazatelů"},

    "Text alignment": {"japanese": "テキストの配置", "korean": "텍스트 정렬", "chinese": "文字对齐", "russian": "Выравнивание текста", "german": "Textausrichtung", "french": "Alignement du texte", "spanish": "Alineación del texto", "italian": "Allineamento del testo", "polish": "Wyrównanie tekstu", "czech": "Zarovnání textu"},
    "Hour format": {"japanese": "時刻形式", "korean": "시간 형식", "chinese": "时间格式", "russian": "Формат времени", "german": "Stundenformat", "french": "Format de l'heure", "spanish": "Formato de hora", "italian": "Formato dell'ora", "polish": "Format godziny", "czech": "Formát hodin"},
    "Alignment": {"japanese": "配置", "korean": "정렬", "chinese": "对齐", "russian": "Выравнивание", "german": "Ausrichtung", "french": "Alignement", "spanish": "Alineación", "italian": "Allineamento", "polish": "Wyrównanie", "czech": "Zarovnání"},
    "Persistent meters": {"japanese": "メーターを常時表示", "korean": "게이지 상시 표시", "chinese": "持续显示条形", "russian": "Постоянные индикаторы", "german": "Dauerhafte Anzeigen", "french": "Jauges permanentes", "spanish": "Medidores persistentes", "italian": "Indicatori persistenti", "polish": "Trwałe wskaźniki", "czech": "Trvalé ukazatele"},

    "Center": {"japanese": "中央", "korean": "가운데", "chinese": "居中", "russian": "По центру", "german": "Mitte", "french": "Centre", "spanish": "Centro", "italian": "Centro", "polish": "Środek", "czech": "Uprostřed"},
    "12h (3:15)": {"japanese": "12時間制 (3:15)", "korean": "12시간제 (3:15)", "chinese": "12小时制 (3:15)", "russian": "12-часовой (3:15)", "german": "12-Stunden (3:15)", "french": "12 h (3:15)", "spanish": "12 h (3:15)", "italian": "12 ore (3:15)", "polish": "12-godzinny (3:15)", "czech": "12hodinový (3:15)"},
    "12h leading 0 (03:15)": {"japanese": "12時間制 先頭0付き (03:15)", "korean": "12시간제 앞자리 0 (03:15)", "chinese": "12小时制补零 (03:15)", "russian": "12-часовой с ведущим нулём (03:15)", "german": "12-Stunden mit führender 0 (03:15)", "french": "12 h avec zéro initial (03:15)", "spanish": "12 h con cero inicial (03:15)", "italian": "12 ore con zero iniziale (03:15)", "polish": "12-godzinny z zerem na początku (03:15)", "czech": "12hodinový s úvodní nulou (03:15)"},
    "24h": {"japanese": "24時間制", "korean": "24시간제", "chinese": "24小时制", "russian": "24-часовой", "german": "24-Stunden", "french": "24 h", "spanish": "24 h", "italian": "24 ore", "polish": "24-godzinny", "czech": "24hodinový"},
    "24h leading 0": {"japanese": "24時間制 先頭0付き", "korean": "24시간제 앞자리 0", "chinese": "24小时制补零", "russian": "24-часовой с ведущим нулём", "german": "24-Stunden mit führender 0", "french": "24 h avec zéro initial", "spanish": "24 h con cero inicial", "italian": "24 ore con zero iniziale", "polish": "24-godzinny z zerem na początku", "czech": "24hodinový s úvodní nulou"},
    "Auto": {"japanese": "自動", "korean": "자동", "chinese": "自动", "russian": "Автоматически", "german": "Automatisch", "french": "Automatique", "spanish": "Automático", "italian": "Automatico", "polish": "Automatycznie", "czech": "Automaticky"},
    "Always on": {"japanese": "常時オン", "korean": "항상 켬", "chinese": "始终开启", "russian": "Всегда включено", "german": "Immer an", "french": "Toujours activé", "spanish": "Siempre activado", "italian": "Sempre attivo", "polish": "Zawsze włączone", "czech": "Vždy zapnuto"},
    "Always off": {"japanese": "常時オフ", "korean": "항상 끔", "chinese": "始终关闭", "russian": "Всегда выключено", "german": "Immer aus", "french": "Toujours désactivé", "spanish": "Siempre desactivado", "italian": "Sempre spento", "polish": "Zawsze wyłączone", "czech": "Vždy vypnuto"},
}


def build_english():
    fixed = read_fixed_keys()
    english = dict(fixed)
    key_to_text_for_check = {}
    for text in TABLE_TEXTS:
        key = derive_key("SHM_El_", text)
        if key in key_to_text_for_check and key_to_text_for_check[key] != text:
            raise RuntimeError(f"derived key collision: {key!r} from both {key_to_text_for_check[key]!r} and {text!r}")
        key_to_text_for_check[key] = text
        if key in english and english[key] != text:
            raise RuntimeError(f"table-derived key {key!r} collides with a fixed key of different text")
        english[key] = text
    for text in COLOUR_TEXTS:
        english[derive_key("SHM_Col_", text)] = text
    return english, fixed


def build_language(lang, english, fixed):
    out = {}
    for key, text in english.items():
        if key in fixed:
            out[key] = FIXED_TRANSLATIONS[key][lang]
        else:
            src_text = None
            for t in COLOUR_TEXTS:
                if derive_key("SHM_Col_", t) == key:
                    out[key] = COLOUR_TRANSLATIONS[t][lang]
                    break
            if key in out:
                continue
            for t in TABLE_TEXTS:
                if derive_key("SHM_El_", t) == key:
                    src_text = t
                    break
            if src_text is None or src_text not in TABLE_TRANSLATIONS or lang not in TABLE_TRANSLATIONS[src_text]:
                raise RuntimeError(f"missing {lang} translation for key {key!r} (text {text!r})")
            out[key] = TABLE_TRANSLATIONS[src_text][lang]
    return out


def write_translation_file(path, entries):
    lines = []
    for key, text in entries.items():
        escaped = text.replace("\r\n", "\n").replace("\n", "\\n")
        lines.append(f"${key}\t{escaped}")
    body = "\r\n".join(lines) + "\r\n"
    data = b"\xff\xfe" + body.encode("utf-16-le")
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as f:
        f.write(data)


def main():
    english, fixed = build_english()
    out_dir = os.path.join(REPO, "dist", "Interface", "Translations")
    write_translation_file(os.path.join(out_dir, "SkyHudMenu_english.txt"), english)
    print(f"english: {len(english)} keys ({len(fixed)} fixed, {len(english) - len(fixed)} table-derived)")
    for lang in LANGS[1:]:
        translated = build_language(lang, english, fixed)
        write_translation_file(os.path.join(out_dir, f"SkyHudMenu_{lang}.txt"), translated)
        print(f"{lang}: {len(translated)} keys written")


if __name__ == "__main__":
    main()
