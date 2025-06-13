<template>
  <div class="min-h-screen flex flex-col justify-center items-center bg-gradient-to-b from-yellow-50 to-yellow-200 relative">
      <!-- 背景大字 -->
   <div class="absolute inset-0 flex justify-center items-start pt-20 select-none pointer-events-none">
  <span class="text-7xl md:text-9xl font-extrabold text-yellow-300/90 drop-shadow-lg">GoBang</span>
</div>
    <div class="z-10 w-full max-w-md p-8 bg-white/90 rounded-2xl shadow-2xl flex flex-col gap-6">
      <h2 class="text-3xl font-bold text-center text-yellow-700 drop-shadow">注册</h2>
      <form @submit.prevent="handleRegister" class="flex flex-col gap-4">
        <input v-model="username" type="text" placeholder="用户名" class="input" />
        <input v-model="password" type="password" placeholder="密码" class="input" />
        <input v-model="email" type="email" placeholder="邮箱" class="input" />
        <button class="btn-yellow" type="submit">注册</button>
        <div class="flex justify-between text-sm">
          <router-link class="text-blue-500 hover:underline" to="/login">已有账号？去登录</router-link>
        </div>
        <p v-if="error" class="text-red-600 text-sm text-center mt-2">{{ error }}</p>
        <p v-if="success" class="text-green-600 text-sm text-center mt-2">{{ success }}</p>
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
const email = ref('')
const error = ref('')
const success = ref('')
const router = useRouter()

async function handleRegister() {
  error.value = ''
  success.value = ''
  if (!username.value || !password.value || !email.value) {
    error.value = '请填写全部字段'
    return
  }
  const res = await post('/auth/register', {
    username: username.value,
    password: password.value,
    email: email.value,
  })
  if (res.success) {
    success.value = '注册成功！请前往登录'
    setTimeout(() => router.push('/login'), 1200)
  } else {
    error.value = res.msg || '注册失败'
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
