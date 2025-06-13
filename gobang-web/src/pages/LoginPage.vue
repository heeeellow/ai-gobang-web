<template>
  <div class="min-h-screen flex flex-col justify-center items-center bg-gradient-to-b from-yellow-50 to-yellow-200 relative">
      <!-- 背景大字 -->
   <div class="absolute inset-0 flex justify-center items-start pt-20 select-none pointer-events-none">
  <span class="text-7xl md:text-9xl font-extrabold text-yellow-300/90 drop-shadow-lg">GoBang</span>
</div>
    <div class="z-10 w-full max-w-md p-8 bg-white/90 rounded-2xl shadow-2xl flex flex-col gap-6">
      <h2 class="text-3xl font-bold text-center text-yellow-700 drop-shadow">登录</h2>
      <form @submit.prevent="handleLogin" class="flex flex-col gap-4">
        <input v-model="username" type="text" placeholder="用户名" class="input" />
        <input v-model="password" type="password" placeholder="密码" class="input" />
        <button class="btn-yellow" type="submit">登录</button>
        <div class="flex justify-between text-sm">
          <router-link class="text-blue-500 hover:underline" to="/register">没有账号？去注册</router-link>
        </div>
        <p v-if="error" class="text-red-600 text-sm text-center mt-2">{{ error }}</p>
      </form>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useRouter } from 'vue-router'
import { post } from '@/utils/api'

const username = ref('')
const password = ref('')
const error = ref('')
const router = useRouter()

async function handleLogin() {
  if (!username.value || !password.value) {
    error.value = '请填写全部字段'
    return
  }
  const res = await post('/auth/login', {
    username: username.value,
    password: password.value,
  })
  if (res.success) {
    localStorage.setItem('user', JSON.stringify(res.user))
    localStorage.setItem('token', res.token)
    router.push('/lobby')
  } else {
    error.value = res.msg || '用户名或密码错误'
    password.value = ''
  }
}
</script>

<style scoped>
.input {
  @apply p-3 rounded-lg border border-gray-300 focus:outline-none focus:ring-2 focus:ring-yellow-400 text-lg bg-white;
}
.btn-yellow {
  @apply w-full p-3 bg-yellow-400 hover:bg-yellow-500 text-white font-bold rounded-xl shadow transition;
}
</style>
