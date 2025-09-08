#!/usr/bin/bash

if [ -n "$SUDO_USER" ]; then
    USER_HOME=$(eval echo ~$SUDO_USER)
else
    USER_HOME=$HOME
fi

rm -rf $USER_HOME/.watermelon

JOBS=${JOBS:-16}
BUILD_TYPE=${BUILD_TYPE:-Release}

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() {
  echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
  echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
  echo -e "${RED}[ERROR]${NC} $1"
}

log_step() {
  echo -e "${BLUE}[STEP]${NC} $1"
}

check_error() {
  if [ $? -ne 0 ]; then
      log_error "$1 failed!"
      exit 1
  fi
}

build_project() {
  local project_name="$1"
  local project_dir="$2"
  
  log_step "Building $project_name..."
  log_info "Project directory: $project_dir"
  
  if [ ! -d "$project_dir" ]; then
      log_error "Directory $project_dir does not exist!"
      exit 1
  fi
  
  cd "$project_dir"
  check_error "Failed to enter directory $project_dir"
  
  rm -rf ./build
  mkdir -p build
  cd build
  
  log_info "Configuring $project_name with CMake..."
  cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..
  check_error "CMake configuration for $project_name"
  
  log_info "Compiling $project_name with $JOBS jobs..."
  make -j"$JOBS"
  check_error "Compilation for $project_name"
  
  log_info "Installing $project_name..."
  make install
  check_error "Installation for $project_name"
  
  cd - > /dev/null
  
  log_info "$project_name build completed successfully!"
  echo
}

main() {
  local start_time=$(date +%s)
  
  log_info "Starting build process..."
  log_info "Build type: $BUILD_TYPE"
  log_info "Jobs: $JOBS"
  echo
  
  for tool in cmake make; do
      if ! command -v "$tool" &> /dev/null; then
          log_error "$tool is not installed or not in PATH"
          exit 1
      fi
  done
  
  local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
  local current_dir="$(pwd)"
  
  log_info "Script directory: $script_dir"
  log_info "Current working directory: $current_dir"
  
  local root_dir="$script_dir"
  local opt_dir="$script_dir/opt"
  local gc_dir="$script_dir/gc"
  
  for dir in "$opt_dir" "$gc_dir"; do
      if [ ! -d "$dir" ]; then
          log_warn "Directory $dir does not exist, skipping..."
      fi
  done
  
  build_project "Root Project" "$root_dir"
  
  if [ -d "$opt_dir" ]; then
      build_project "Opt Project" "$opt_dir"
  else
      log_warn "Skipping Opt Project - directory not found: $opt_dir"
  fi
  
  if [ -d "$gc_dir" ]; then
      build_project "GC Project" "$gc_dir"
      opt -S -load-pass-plugin=$USER_HOME/.watermelon/opt/lib_mem2reg_pass.so \
            -load-pass-plugin=$USER_HOME/.watermelon/opt/lib_constant_prop_pass.so\
            -load-pass-plugin=$USER_HOME/.watermelon/opt/lib_dce_pass.so\
            -passes="mem2reg-pass,constant-prop-pass,dce-pass"\
            $USER_HOME/.watermelon/gc/gc.ll -o $USER_HOME/.watermelon/gc/gc.ll
  else
      log_warn "Skipping GC Project - directory not found: $gc_dir"
  fi
  
  cd "$current_dir"
  
  local end_time=$(date +%s)
  local duration=$((end_time - start_time))
  
  log_info "All builds completed successfully!"
  log_info "Total build time: ${duration}s"

  echo
  log_info "Install watermelon compiler to /usr/local/bin !"
  sudo cp ./bin/watermelon /usr/local/bin

}

show_help() {
  echo "Usage: $0 [OPTIONS]"
  echo
  echo "This script will build projects in the following order:"
  echo "  1. Root project (current directory)"
  echo "  2. Opt project (./opt subdirectory)"
  echo "  3. GC project (./gc subdirectory)"
  echo
  echo "Environment Variables:"
  echo "  JOBS        Number of parallel jobs (default: 16)"
  echo "  BUILD_TYPE  CMake build type (default: Release)"
  echo
  echo "Options:"
  echo "  -h, --help  Show this help message"
  echo
  echo "Examples:"
  echo "  $0                    # Build with default settings"
  echo "  JOBS=8 $0            # Build with 8 parallel jobs"
  echo "  BUILD_TYPE=Debug $0   # Debug build"
}

# 参数解析
case "${1:-}" in
  -h|--help)
      show_help
      exit 0
      ;;
  "")
      main
      ;;
  *)
      log_error "Unknown option: $1"
      show_help
      exit 1
      ;;
esac