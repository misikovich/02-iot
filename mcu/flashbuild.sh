#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [ -d "$SCRIPT_DIR/mcu" ]; then
    ROOT_DIR="$SCRIPT_DIR"
    MCU_DIR="$SCRIPT_DIR/mcu"
elif [ -f "$SCRIPT_DIR/Makefile" ] && [ -d "$SCRIPT_DIR/nbproject" ]; then
    ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
    MCU_DIR="$SCRIPT_DIR"
else
    ROOT_DIR="$SCRIPT_DIR"
    MCU_DIR="$SCRIPT_DIR/mcu"
fi

MCU="${MCU:-24EP128MC206}"
MCU="${MCU#PIC}"
TOOL="${PROGRAMMER_TOOL:-PK4}"
TOOL="${TOOL#TP}"
HEX_FILE="${HEX_FILE:-$MCU_DIR/dist/default/production/mcu.production.hex}"
POWER_TARGET="${POWER_TARGET:-1}"
VOLTAGE="${VOLTAGE:-}"
DRY_RUN=0
SKIP_BUILD=0
CLEAN_BUILD=0

usage() {
    cat <<USAGE
Usage: $(basename "$0") [options] [extra ipecmd args...]

Options:
  --clean               Clean before building
  --skip-build          Skip build step and flash existing hex directly
  --hex PATH            Path to hex file (default: $HEX_FILE)
  --tool TOOL           Programmer tool (default: PK4)
  --mcu MCU             Target microcontroller (default: 24EP128MC206)
  --power               Power target board from PICkit (default: enabled)
  --no-power            Do not power target board from PICkit (for externally powered boards)
  --voltage VOLTS       Supply specific voltage with -W (e.g. 3.3)
  --dry-run             Print build and ipecmd commands without running flash
  -h, --help            Show this help message

Environment variables:
  MCU                   Target MCU (e.g. 24EP128MC206)
  PROGRAMMER_TOOL       Tool name (e.g. PK4)
  HEX_FILE              Path to hex file
  POWER_TARGET          1 to power target from tool, 0 for externally powered
  VOLTAGE               Voltage value for -W (e.g. 3.3)
  IPECMD                Path to ipecmd.sh executable
USAGE
}

require_value() {
    if [ "$#" -lt 2 ]; then
        printf 'Missing value for %s\n' "$1" >&2
        exit 1
    fi
}

find_ipecmd() {
    if [ -n "${IPECMD:-}" ]; then
        printf '%s\n' "$IPECMD"
        return 0
    fi

    local candidate
    for candidate in \
        /opt/microchip/mplabx/*/mplab_platform/mplab_ipe/ipecmd.sh \
        /opt/microchip/mplabx/*/mplab_platform/bin/ipecmd.sh \
        "$HOME"/mplab/mplabx/mplab_platform/mplab_ipe/ipecmd.sh \
        "$HOME"/mplab/mplabx/mplab_platform/bin/ipecmd.sh \
        "$HOME"/mplab/mplabx/*/mplab_platform/mplab_ipe/ipecmd.sh \
        "$HOME"/mplab/mplabx/*/mplab_platform/bin/ipecmd.sh \
        "$HOME"/mplabx/*/mplab_platform/mplab_ipe/ipecmd.sh \
        "$HOME"/.mplab/*/mplab_platform/mplab_ipe/ipecmd.sh
    do
        if [ -x "$candidate" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    if command -v ipecmd.sh >/dev/null 2>&1; then
        command -v ipecmd.sh
        return 0
    fi
    if command -v ipecmd >/dev/null 2>&1; then
        command -v ipecmd
        return 0
    fi

    return 1
}

EXTRA_ARGS=()
while [ "$#" -gt 0 ]; do
    case "$1" in
        --clean)
            CLEAN_BUILD=1
            shift
            ;;
        --skip-build|--no-build)
            SKIP_BUILD=1
            shift
            ;;
        --hex)
            require_value "$@"
            HEX_FILE="$2"
            shift 2
            ;;
        --hex=*)
            HEX_FILE="${1#*=}"
            shift
            ;;
        --tool)
            require_value "$@"
            TOOL="${2#TP}"
            shift 2
            ;;
        --tool=*)
            TOOL="${1#*=}"
            TOOL="${TOOL#TP}"
            shift
            ;;
        --mcu)
            require_value "$@"
            MCU="${2#PIC}"
            shift 2
            ;;
        --mcu=*)
            MCU="${1#*=}"
            MCU="${MCU#PIC}"
            shift
            ;;
        --power)
            POWER_TARGET=1
            shift
            ;;
        --no-power)
            POWER_TARGET=0
            shift
            ;;
        --voltage)
            require_value "$@"
            VOLTAGE="$2"
            POWER_TARGET=1
            shift 2
            ;;
        --voltage=*)
            VOLTAGE="${1#*=}"
            POWER_TARGET=1
            shift
            ;;
        --dry-run)
            DRY_RUN=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            EXTRA_ARGS+=("$@")
            break
            ;;
        *)
            EXTRA_ARGS+=("$1")
            shift
            ;;
    esac
done

if [ "$SKIP_BUILD" != "1" ]; then
    printf '==> Building MCU firmware in %s...\n' "$MCU_DIR"
    if [ "$CLEAN_BUILD" = "1" ]; then
        make -C "$MCU_DIR" clean
    fi
    make -C "$MCU_DIR"
fi

if [ ! -f "$HEX_FILE" ]; then
    printf 'Error: Missing HEX file: %s\n' "$HEX_FILE" >&2
    exit 1
fi

if ! IPECMD_PATH="$(find_ipecmd)"; then
    printf 'Error: Could not find ipecmd. Install MPLAB X IPE or set IPECMD=/path/to/ipecmd.sh\n' >&2
    exit 1
fi

CMD=(
    "$IPECMD_PATH"
    "-TP$TOOL"
    "-P$MCU"
    "-F$HEX_FILE"
    -M
    -OL
)

if [ "$POWER_TARGET" = "1" ]; then
    if [ -n "$VOLTAGE" ]; then
        CMD+=("-W$VOLTAGE")
    else
        CMD+=(-W)
    fi
fi

CMD+=("${EXTRA_ARGS[@]}")

printf '==> Flashing %s to %s via %s...\n' "$HEX_FILE" "$MCU" "$TOOL"
printf '%q ' "${CMD[@]}"
printf '\n'

if [ "$DRY_RUN" = "1" ]; then
    exit 0
fi

exec "${CMD[@]}"
